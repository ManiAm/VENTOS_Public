/****************************************************************************/
/// @file    SniffUSB.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Dec 2015
///
/****************************************************************************/
// VENTOS, Vehicular Network Open Simulator; see http:?
// Copyright (C) 2013-2015
/****************************************************************************/
//
// This file is part of VENTOS.
// VENTOS is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include <02_SniffUSB.h>
#include "TraCI_Commands.h"
#include <fstream>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

// un-defining ev!
// why? http://stackoverflow.com/questions/24103469/cant-include-the-boost-filesystem-header
#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

namespace VENTOS {

libusb_device_handle * SniffUSB::hotPlugHandle = NULL;

Define_Module(VENTOS::SniffUSB);

SniffUSB::~SniffUSB()
{

}


void SniffUSB::initialize(int stage)
{
    if(stage == 0)
    {
        on = par("on").boolValue();
        if(!on)
            return;

        listUSBdevices = par("listUSBdevices").boolValue();
        hotPlug = par("hotPlug").boolValue();

        target_vendor_id = (uint16_t) par("target_vendor_id").longValue();
        target_product_id = (uint16_t) par("target_product_id").longValue();
        target_interfaceNumber = (uint16_t) par("target_interfaceNumber").longValue();
        target_interruptEP = (unsigned char) par("target_interruptEP").longValue();

        // start a libusb session
        int r = libusb_init(&ctx); // initialize a library session
        if(r < 0)
            error("failed to initialize libusb: %s\n", libusb_error_name(r));

        libusb_set_debug(ctx, 3); // set verbosity level to 3, as suggested in the documentation

        // register signals
        Signal_executeFirstTS = registerSignal("executeFirstTS");
        simulation.getSystemModule()->subscribe("executeFirstTS", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        simulation.getSystemModule()->subscribe("executeEachTS", this);

        USBevents = new cMessage("USBevents", KIND_TIMER);
        USBInterrupt = new cMessage("USBInterrupt", KIND_TIMER);

        if(listUSBdevices)
        {
            std::cout << std::endl << ">>> USB devices found on this machine: \n";
            bool detail = par("listUSBdevicesDetailed").boolValue();
            getUSBdevices(detail, "");
        }
    }
}


void SniffUSB::finish()
{
    if(!on)
        return;

    // release the claimed interface
    if(dev_handle != NULL)
    {
        int r = libusb_release_interface(dev_handle, target_interfaceNumber);
        if(r != 0)
            error("Cannot Release Interface! %s", libusb_error_name(r));

        std::cout << "Interface " << target_interfaceNumber << " Released." << endl;

        libusb_close(dev_handle); //close the device we opened
    }

    if(ctx != NULL)
        libusb_exit(ctx); //close the libusb session
}


void SniffUSB::handleMessage(cMessage *msg)
{
    if (msg == USBevents)
    {
        timeval to;
        to.tv_sec = 1;
        int rc = libusb_handle_events_timeout_completed(ctx, &to, NULL);

        //int rc = libusb_handle_events(ctx);
        if (rc < 0)
        {
            printf("libusb_handle_events() failed: %s\n", libusb_error_name(rc));
            std::cout.flush();
        }

        scheduleAt(simTime() + 0.5, USBevents);
    }
    else if(msg == USBInterrupt)
    {
        if(dev_handle != NULL && EPPacketSize > 0)
        {
            int actual_length = 0;
            unsigned char data_in[65535];

            libusb_interrupt_transfer(dev_handle,
                    target_interruptEP,
                    data_in, EPPacketSize, &actual_length, 5);

            if(actual_length > 0)
            {
                printf("Received %u bytes via interrupt transfer:\n", actual_length);
                for(int i = 0; i < actual_length; i++)
                    printf("%02x ",data_in[i]);

                printf("\n");
                std::cout.flush();
            }
        }

        scheduleAt(simTime() + 0.2, USBInterrupt);
    }
}


void SniffUSB::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeEachTS)
    {
        SniffUSB::executeEachTimestep();
    }
    else if(signalID == Signal_executeFirstTS)
    {
        SniffUSB::executeFirstTimeStep();
    }
}


void SniffUSB::executeFirstTimeStep()
{

}


void SniffUSB::executeEachTimestep()
{
    // run this code only once
    static bool wasExecuted = false;
    if (on && !wasExecuted)
    {
        if(hotPlug)
            EnableHotPlug();

        startSniffing();

        wasExecuted = true;
    }
}


// listUSBdevicesDetailed: shows 'Configuration Descriptor' for each device
// productID: looks only for this productID
void SniffUSB::getUSBdevices(bool listUSBdevicesDetailed, std::string productID)
{
    // make sure ctx is not NULL
    if(ctx == NULL)
    {
        // start a libusb session
        int r = libusb_init(&ctx); // initialize a library session
        if(r < 0)
            error("failed to initialize libusb: %s\n", libusb_error_name(r));

        libusb_set_debug(ctx, 3); // set verbosity level to 3, as suggested in the documentation
    }

    libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
    ssize_t cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
    if(cnt < 0)
        std::cout << "Get Device Error" << endl;

    // iterate over each found USB device
    for(ssize_t i = 0; i < cnt; i++)
    {
        // get device descriptor first!
        libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(devs[i], &desc);
        if (r < 0)
            error("failed to get device descriptor");

        // decode vendorID and productID
        std::vector<std::string> names = USBidTostr(desc.idVendor, desc.idProduct);

        std::string pID = names[1];
        boost::algorithm::to_lower(pID);
        boost::algorithm::to_lower(productID);
        if( productID != "" && pID.find(productID) == std::string::npos )
            continue;

        // decode class name
        std::string className = "?";
        auto res = USBclass.find(desc.bDeviceClass);
        if(res != USBclass.end())
            className = res->second;

        printf("Device %d connected to Port %d of Bus %d with Speed %d (%s) \n",
                libusb_get_device_address(devs[i]),
                libusb_get_port_number(devs[i]),
                libusb_get_bus_number(devs[i]),
                libusb_get_device_speed(devs[i]), USBspeedTostr(libusb_get_device_speed(devs[i])).c_str() );

        printf("    Device Descriptor --> ");
        // in windows, The USB driver stack uses bcdDevice, along with idVendor and idProduct, to generate hardware and compatible IDs for the device.
        // You can view the those identifiers in Device Manager.
        printf("VendorID: %04x (%s), ProductID: %04x (%s), Release Number: %u \n", desc.idVendor, names[0].c_str(), desc.idProduct, names[1].c_str(), desc.bcdDevice);
        printf("                          Device Class: %x (%s), Device Subclass: %x \n", desc.bDeviceClass, className.c_str(), desc.bDeviceSubClass);
        printf("                          Manufacturer: %u, Product: %u, Serial: %u \n", desc.iManufacturer, desc.iProduct, desc.iSerialNumber);
        printf("                          USB Version: %s \n", USBversionTostr(desc.bcdUSB).c_str());

        if(listUSBdevicesDetailed)
        {
            printf("    Configuration Descriptor (#%-2u) --> \n", desc.bNumConfigurations);
            printConfDesc(devs[i]);
        }

        printf("\n");
    }

    std::cout.flush();

    libusb_free_device_list(devs, 1); //free the list, unref the devices in it
}


std::vector<std::string> SniffUSB::USBidTostr(uint16_t idVendor, uint16_t idProduct)
{
    if(USBids.empty())
    {
        // usbids is downloaded from http://www.linux-usb.org/usb.ids
        boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        boost::filesystem::path usbids_FullPath = VENTOS_FullPath / "application/sniffing/sniff_usb_ids";

        std::ifstream in(usbids_FullPath.string().c_str());
        if(in.fail())
            error("cannot open file sniff_usb_ids at %s", usbids_FullPath.string().c_str());

        std::string line;
        usb_vender *v = NULL;
        while(getline(in, line))
        {
            boost::char_separator<char> sep("\x09", "", boost::keep_empty_tokens);
            boost::tokenizer< boost::char_separator<char> > tokens(line, sep);

            int vendor_id = -1;
            std::string vendor_name = "?";
            int device_id = -1;
            std::string device_name = "?";

            for(auto beg = tokens.begin(); beg != tokens.end(); ++beg)
            {
                try
                {
                    // vendor
                    if(*beg != "" && *beg != "#")
                    {
                        std::size_t found = (*beg).find(' ');
                        if (found != std::string::npos)
                        {
                            // vendor_id will be in decimal
                            vendor_id = std::stoi((*beg).substr(0, found), nullptr, 16);
                            vendor_name = (*beg).substr(found+2);
                        }

                        v = new usb_vender(vendor_id, vendor_name);
                        if(USBids.find(*v) == USBids.end())
                            USBids[*v] = std::vector<usb_device>();
                        else
                            error("vender %d exists more than once!", vendor_id);
                    }
                    // device
                    else if(*beg == "")
                    {
                        // advance iterator forward
                        std::advance (beg,1);

                        std::size_t found = (*beg).find(' ');
                        if (found != std::string::npos)
                        {
                            // device_id will be in decimal
                            device_id = std::stoi((*beg).substr(0, found), nullptr, 16);
                            device_name = (*beg).substr(found+2);

                            usb_device *d = new usb_device(device_id, device_name);
                            auto it = USBids.find(*v);
                            if(it != USBids.end())
                                (it->second).push_back(*d);
                            else
                                error("vendor does not exist for %d", device_id);
                        }
                    }
                }
                catch(std::exception e)
                {
                    // std::cout << "Error parsing sniff_usb_ids file, " << e.what() << std::endl << std::flush;
                }
            }
        }
    }

    // vender_name is not important
    usb_vender *v = new usb_vender(idVendor, "");
    auto it = USBids.find(*v);

    // vendor_id not found
    if(it == USBids.end())
    {
        std::vector<std::string> ret = { "", "" };
        return ret;
    }
    // vendor_id found
    else
    {
        for(auto x : it->second)
        {
            if(x.id == idProduct)
            {
                std::vector<std::string> ret = { it->first.name, x.name };
                return ret;
            }
        }

        std::vector<std::string> ret = { it->first.name, "?" };
        return ret;
    }
}


std::string SniffUSB::USBversionTostr(uint16_t version)
{
    if(version == 256)
        return "1.0";
    else if(version == 272)
        return "1.1";
    else if(version == 512)
        return "2.0";
    else if(version == 768)
        return "3.0";
    else
        return "?";
}


std::string SniffUSB::USBspeedTostr(int speed)
{
    if(speed == 0)
        return "SPEED_UNKNOWN";
    else if(speed == 1)
        return "SPEED_LOW";
    else if(speed == 2)
        return "SPEED_FULL";
    else if(speed == 3)
        return "SPEED_HIGH";
    else if(speed == 4)
        return "SPEED_SUPER";
    else
        return "?";
}


void SniffUSB::printConfDesc(libusb_device *dev)
{
    libusb_config_descriptor *config;
    libusb_get_config_descriptor(dev, 0, &config);

    const libusb_interface *inter;
    const libusb_interface_descriptor *interdesc;
    const libusb_endpoint_descriptor *epdesc;

    printf("                          Interface Descriptor --> \n");

    for(uint8_t i=0; i<config->bNumInterfaces; i++)
    {
        inter = &config->interface[i];
        for(int j=0; j<inter->num_altsetting; j++)
        {
            interdesc = &inter->altsetting[j];

            std::string className = "?";
            auto res = USBclass.find(interdesc->bInterfaceClass);
            if(res != USBclass.end())
                className = res->second;

            char interfacePath[64];
            snprintf(interfacePath, sizeof(interfacePath), "%04x:%04x:%02x",
                    libusb_get_bus_number(dev),
                    libusb_get_device_address(dev),
                    interdesc->bInterfaceNumber);
            interfacePath[sizeof(interfacePath)-1] = '\0';

            printf("                                      Interface %-2d --> Class: %x (%s), Subclass: %x, Path: %s \n",
                    interdesc->bInterfaceNumber,
                    interdesc->bInterfaceClass,
                    className.c_str(),
                    interdesc->bInterfaceSubClass,
                    strdup(interfacePath));

            printf("                                                End Points --> \n");

            for(uint8_t k=0; k<interdesc->bNumEndpoints; k++)
            {
                epdesc = &interdesc->endpoint[k];
                printf("                                                              Address: %-3x (%-16s), Attribute: %-3u (%s), Max Packet Size: %u \n",
                        epdesc->bEndpointAddress, EPAddTostr(epdesc->bEndpointAddress).c_str(),
                        epdesc->bmAttributes, EPAttTostr(epdesc->bmAttributes).c_str(),
                        epdesc->wMaxPacketSize);
            }
        }

        std::cout << std::endl;
    }

    libusb_free_config_descriptor(config);
}


std::string SniffUSB::EPAddTostr(uint8_t addr)
{
    int EPnumber = (addr & 0x1F);
    std::string EPnumberStr = std::to_string(EPnumber);

    int direction = (addr & 0x80);
    std::string directionStr;
    if(direction == LIBUSB_ENDPOINT_OUT)     /* Out: host-to-device */
        directionStr = "ENDPOINT_OUT";
    else if(direction == LIBUSB_ENDPOINT_IN) /* In: device-to-host */
        directionStr = "ENDPOINT_IN";

    return (EPnumberStr + ", " + directionStr);
}


std::string SniffUSB::EPAttTostr(uint8_t attValue)
{
    std::ostringstream att;

    int transferType = (attValue & 0x03);
    switch(transferType)
    {
    case LIBUSB_TRANSFER_TYPE_CONTROL:
        att << "Control";
        break;
    case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
        att << "Isochronous";
        break;
    case LIBUSB_TRANSFER_TYPE_BULK:
        att << "Bulk";
        break;
    case LIBUSB_TRANSFER_TYPE_INTERRUPT:
        att << "Interrupt";
        break;
    }

    if(transferType == 1)
    {
        int syncType = ((attValue & 0x0c) >> 2);
        switch(syncType)
        {
        case LIBUSB_ISO_SYNC_TYPE_NONE:
            att << ", No Sync";
            break;
        case LIBUSB_ISO_SYNC_TYPE_ASYNC:
            att << ", Async";
            break;
        case LIBUSB_ISO_SYNC_TYPE_ADAPTIVE:
            att << ", Adaptive";
            break;
        case LIBUSB_ISO_SYNC_TYPE_SYNC:
            att << ", Sync";
            break;
        }

        int usageType = ((attValue & 0x30) >> 4);
        switch(usageType)
        {
        case LIBUSB_ISO_USAGE_TYPE_DATA:
            att << ", Data Endpoint";
            break;
        case LIBUSB_ISO_USAGE_TYPE_FEEDBACK:
            att << ", Feedback Endpoint";
            break;
        case LIBUSB_ISO_USAGE_TYPE_IMPLICIT:
            att << ", Explicit Feedback Data Endpoint";
            break;
        case 3:
            att << ", Reserved";
            break;
        }
    }

    return att.str();
}


void SniffUSB::EnableHotPlug()
{
    int vendor_id = 1133;
    int product_id = 50475;
    int class_id = LIBUSB_HOTPLUG_MATCH_ANY;
    int rc;
    libusb_hotplug_callback_handle hp[2];

    if (!libusb_has_capability (LIBUSB_CAP_HAS_HOTPLUG))
    {
        libusb_exit (NULL);
        error("Hotplug capabilities are not supported on this platform");
    }

    rc = libusb_hotplug_register_callback(ctx, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, (libusb_hotplug_flag)0, vendor_id, product_id, class_id, hotplug_callback, NULL, &hp[0]);
    if (rc < 0)
    {
        libusb_exit (NULL);
        error("Error registering callback 0");
    }

    std::cout << "HOTPLUG_EVENT_DEVICE_ARRIVED registered!" << std::endl;
    std::cout.flush();

    rc = libusb_hotplug_register_callback(ctx, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, (libusb_hotplug_flag)0, vendor_id, product_id, class_id, hotplug_callback_detach, NULL, &hp[1]);
    if (rc < 0)
    {
        libusb_exit (NULL);
        error("Error registering callback 1");
    }

    std::cout << "HOTPLUG_EVENT_DEVICE_LEFT registered!" << std::endl;
    std::cout.flush();

    // start monitoring events
    scheduleAt(simTime() + 0.5, USBevents);
}


int SniffUSB::hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    struct libusb_device_descriptor desc;
    int rc;

    rc = libusb_get_device_descriptor(dev, &desc);
    if (LIBUSB_SUCCESS != rc)
        throw cRuntimeError("Error getting device descriptor");

    printf("Device %04x:%04x attached ...\n", desc.idVendor, desc.idProduct);
    std::cout.flush();

    if (hotPlugHandle)
    {
        libusb_close(hotPlugHandle);
        hotPlugHandle = NULL;
    }

    rc = libusb_open(dev, &hotPlugHandle);
    if (LIBUSB_SUCCESS != rc)
        throw cRuntimeError("Error opening device");

    return 0;
}


int LIBUSB_CALL SniffUSB::hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    printf ("Device detached. \n");
    std::cout.flush();

    if (hotPlugHandle)
    {
        libusb_close(hotPlugHandle);
        hotPlugHandle = NULL;
    }

    return 0;
}


void SniffUSB::startSniffing()
{
    // make sure ctx is not NULL
    if(ctx == NULL)
        error("No libusb session is established!");

    // open the USB device that we are looking for
    dev_handle = libusb_open_device_with_vid_pid(ctx, target_vendor_id, target_product_id);
    if(dev_handle == NULL)
        error("Cannot open USB device %04x:%04x. Check 'target_vendor_id' and 'target_product_id'", target_vendor_id, target_product_id);

    printf("Device %04x:%04x opened ...\n", target_vendor_id, target_product_id);

    std::cout.flush();

    // find out if kernel driver is attached
    int r = libusb_kernel_driver_active(dev_handle, target_interfaceNumber);
    if(r == 1)
    {
        std::cout << "Kernel Driver Active" << endl;

        // detach it (seems to be just like rmmod?)
        if(libusb_detach_kernel_driver(dev_handle, target_interfaceNumber) == 0)
            std::cout << "Kernel Driver Detached!" << endl;

        int cfg = -1;
        libusb_get_configuration(dev_handle, &cfg);
        if (cfg != 1)
        {
            std::cout << "Setting device configuration to 1." << std::endl;
            r = libusb_set_configuration(dev_handle, 1);
            if (r < 0)
                error("libusb_set_configuration error: %s", libusb_error_name(r));
        }
    }

    // claim interface of device
    r = libusb_claim_interface(dev_handle, target_interfaceNumber);
    if(r < 0)
        error("Cannot claim interface! %s", libusb_error_name(r));

    std::cout << "Interface " << target_interfaceNumber << " Claimed." << endl;

    libusb_device* thisDev = libusb_get_device(dev_handle);
    EPPacketSize = libusb_get_max_packet_size(thisDev, target_interruptEP);
    if(EPPacketSize < 0)
        error("libusb_get_max_packet_size failed!");

    // start getting data events
    scheduleAt(simTime() + 0.00001, USBInterrupt);
}


void SniffUSB::bulk(libusb_device_handle *devh)
{
    //    // Request data from the device
    //    unsigned char data_in[200] = { 0 };
    //    int bytes_received = libusb_control_transfer(
    //            dev_handle,
    //            LIBUSB_ENDPOINT_IN | LIBUSB_RECIPIENT_ENDPOINT, // request_type
    //            LIBUSB_REQUEST_GET_STATUS,  // bRequest
    //            0/*0x0100*/,  // wValue
    //            0x83,  // wIndex
    //            data_in,
    //            200,  // wLength
    //            1000);   // TIMEOUT_MS
    //
    //    if (bytes_received < 0)
    //        error("Error receiving descriptor");
    //
    //    printf("Received %u bytes from the USB device:\n", bytes_received);
    //    for(int i = 0; i < bytes_received; i++)
    //        printf("%02x ", data_in[i]);
    //    printf("\n");
    //    std::cout.flush();




    //    // data to write
    //    unsigned char *data = new unsigned char[4];
    //    data[0] = 'a'; data[1] = 'b'; data[2] = 'c'; data[3] = 'd';
    //
    //    std::cout << "Writing Data ... ";
    //
    //    // my device's out endpoint was 2
    //    int actual; //used to find out how many bytes were written
    //    r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT), data, 4, &actual, 0);
    //    if(r == 0 && actual == 4)
    //        std::cout << "Successful!" << endl;
    //    else
    //        std::cout << "Error!" << endl;





    //    // Request data from the device
    //    static const int MAX_CONTROL_IN_TRANSFER_SIZE = 2;  // With firmware support, transfers can be > the endpoint's max packet size
    //    unsigned char data_in[MAX_CONTROL_IN_TRANSFER_SIZE];
    //    int bytes_received = libusb_control_transfer(
    //            dev_handle,
    //            LIBUSB_ENDPOINT_IN, // bmRequestType (direction of the request, type of request and designated recipient)
    //            LIBUSB_REQUEST_GET_DESCRIPTOR,    // bRequest (HID_GET_REPORT)
    //            (LIBUSB_DT_STRING << 8) | 0x00,  // value field. HID_REPORT_TYPE_FEATURE = 0x03
    //            0,   // INTERFACE_NUMBER
    //            data_in,
    //            MAX_CONTROL_IN_TRANSFER_SIZE,
    //            5000);  // TIMEOUT_MS
    //
    //    if (bytes_received < 0)
    //        error("Error receiving Feature report: %s", libusb_error_name(bytes_received));
    //
    //    printf("Feature report data received:\n");
    //    for(int i = 0; i < bytes_received; i++)
    //        printf("%02x ", data_in[i]);
    //    printf("\n");

}

}

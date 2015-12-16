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

#include <SniffUSB.h>
#include <boost/tokenizer.hpp>

namespace VENTOS {

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
        showConfDesc = par("showConfDesc").boolValue();

        getUSBids();
        getUSBdevices();
        startSniffing();
    }
}


void SniffUSB::finish()
{

}


void SniffUSB::handleMessage(cMessage *msg)
{

}


void SniffUSB::getUSBids()
{
    // usbids is downloaded from http://www.linux-usb.org/usb.ids
    boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
    boost::filesystem::path usbids_FullPath = VENTOS_FullPath / "sniff_usb_ids";

    std::ifstream in(usbids_FullPath.string().c_str());
    if(in == NULL)
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
                // do nothing!
            }
        }
    }
}


std::vector<std::string> SniffUSB::USBidToName(uint16_t idVendor, uint16_t idProduct)
{
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

        std::vector<std::string> ret = { it->first.name, "" };
        return ret;
    }
}


const char* SniffUSB::USBversion(uint16_t version)
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


void SniffUSB::getUSBdevices()
{
    libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
    libusb_context *ctx = NULL; //a libusb session

    int r = libusb_init(&ctx); //initialize a library session
    if(r < 0)
        error("Init Error %d", r);

    libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation
    ssize_t cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
    if(cnt < 0)
        std::cout << "Get Device Error" << endl;

    if(listUSBdevices)
    {
        std::cout << std::endl << "found " << cnt << " USB devices on this machine: \n\n";

        for(ssize_t i = 0; i < cnt; i++)
            printdev(devs[i]); //print specs of this device
    }

    std::cout.flush();

    // open the USB device that we are looking for
    libusb_device_handle *dev_handle = libusb_open_device_with_vid_pid(ctx, 1133, 49242);
    if(dev_handle == NULL)
        error("Cannot open device \n");
    else
        std::cout << "Device Opened \n";


    // todo: read/write to USB

    libusb_free_device_list(devs, 1); //free the list, unref the devices in it
    libusb_exit(ctx); //close the session
}


void SniffUSB::printdev(libusb_device *dev)
{
    printf("Device %d connected to Port %d of Bus %d \n", libusb_get_device_address(dev), libusb_get_port_number(dev), libusb_get_bus_number(dev));

    printf("    Device Descriptor --> ");
    libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0)
        error("failed to get device descriptor");

    std::vector<std::string> names = USBidToName(desc.idVendor, desc.idProduct);
    printf("VendorID: %u (%s), ProductID: %u (%s) \n", desc.idVendor, names[0].c_str(), desc.idProduct, names[1].c_str());
    printf("                          Device Class: %u, Device Subclass: %u \n", desc.bDeviceClass, desc.bDeviceSubClass);
    printf("                          Manufacturer: %u, Product: %u, Serial: %u \n", desc.iManufacturer, desc.iProduct, desc.iSerialNumber);
    printf("                          USB Version: %s \n", USBversion(desc.bcdUSB));

    if(showConfDesc)
    {
        printf("    Configuration Descriptor (#%-2u) --> \n", desc.bNumConfigurations);
        libusb_config_descriptor *config;
        libusb_get_config_descriptor(dev, 0, &config);

        const libusb_interface *inter;
        const libusb_interface_descriptor *interdesc;
        const libusb_endpoint_descriptor *epdesc;

        printf("                                       Interface Descriptor --> \n");

        for(uint8_t i=0; i<config->bNumInterfaces; i++)
        {
            inter = &config->interface[i];
            for(int j=0; j<inter->num_altsetting; j++)
            {
                interdesc = &inter->altsetting[j];
                printf("                                                                Interface %-2d --> \n", interdesc->bInterfaceNumber);
                for(uint8_t k=0; k<interdesc->bNumEndpoints; k++)
                {
                    epdesc = &interdesc->endpoint[k];
                    printf("                                                                                 Descriptor Type: %d, EP Address: %d \n", epdesc->bDescriptorType, epdesc->bEndpointAddress);
                }
            }

            std::cout << std::endl;
        }

        libusb_free_config_descriptor(config);
    }
}


void SniffUSB::startSniffing()
{

}


}

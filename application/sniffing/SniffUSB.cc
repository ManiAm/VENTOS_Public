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

        startSniffing();
    }
}


void SniffUSB::finish()
{

}


void SniffUSB::handleMessage(cMessage *msg)
{

}


void SniffUSB::startSniffing()
{
    libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
    libusb_context *ctx = NULL; //a libusb session

    int r = libusb_init(&ctx); //initialize a library session
    if(r < 0)
        error("Init Error %d", r);

    libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation
    ssize_t cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
    if(cnt < 0)
        std::cout << "Get Device Error" << endl; //there was an error

    std::cout << cnt << " Devices in list." << endl; //print total number of usb devices
    ssize_t i; //for iterating through the list
    for(i = 0; i < cnt; i++)
        printdev(devs[i]); //print specs of this device

    libusb_free_device_list(devs, 1); //free the list, unref the devices in it
    libusb_exit(ctx); //close the session
}


void SniffUSB::printdev(libusb_device *dev)
{
    libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0)
        error("failed to get device descriptor");

    std::cout << "Number of possible configurations: " << (int)desc.bNumConfigurations << "  ";
    std::cout << "Device Class: " << (int)desc.bDeviceClass << "  ";
    std::cout << "VendorID: " << desc.idVendor << "  ";
    std::cout << "ProductID: " << desc.idProduct << endl;
    libusb_config_descriptor *config;
    libusb_get_config_descriptor(dev, 0, &config);
    std::cout << "Interfaces: " << (int)config->bNumInterfaces << " ||| ";
    const libusb_interface *inter;
    const libusb_interface_descriptor *interdesc;
    const libusb_endpoint_descriptor *epdesc;

    for(int i=0; i<(int)config->bNumInterfaces; i++)
    {
        inter = &config->interface[i];
        std::cout << "Number of alternate settings: " << inter->num_altsetting << " | ";
        for(int j=0; j<inter->num_altsetting; j++)
        {
            interdesc = &inter->altsetting[j];
            std::cout << "Interface Number: " << (int)interdesc->bInterfaceNumber << " | ";
            std::cout << "Number of endpoints: " << (int)interdesc->bNumEndpoints << " | ";
            for(int k=0; k<(int)interdesc->bNumEndpoints; k++)
            {
                epdesc = &interdesc->endpoint[k];
                std::cout << "Descriptor Type: " <<(int)epdesc->bDescriptorType << " | ";
                std::cout << "EP Address: " <<(int)epdesc->bEndpointAddress << " | ";
            }
        }
    }

    std::cout << std::endl << std::endl << std::endl;
    libusb_free_config_descriptor(config);
}


}

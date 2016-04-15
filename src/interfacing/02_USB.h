/****************************************************************************/
/// @file    USB.h
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

#ifndef SNIFFUSB
#define SNIFFUSB

#include <BaseApplLayer.h>
#include "TraCICommands.h"
#include <unordered_map>
#include <libusb-1.0/libusb.h>

namespace VENTOS {

class usb_vender
{
public:
    int id;
    std::string name;

    usb_vender(int x, std::string y)
    {
        this->id = x;
        this->name = y;
    }

    bool operator<( const usb_vender& other) const
    {
        if(id < other.id)
            return true;
        else
            return false;
    }
};


class usb_device
{
public:
    int id;
    std::string name;

    usb_device(int x, std::string y)
    {
        this->id = x;
        this->name = y;
    }
};


class USB : public BaseApplLayer
{
public:
    virtual ~USB();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);
    virtual void receiveSignal(cComponent *, simsignal_t, long);

public:
    void getUSBdevices(bool);

private:
    void executeFirstTimeStep();
    void executeEachTimestep();

    std::vector<std::string> USBidTostr(uint16_t, uint16_t);
    std::string USBversionTostr(uint16_t version);
    std::string USBspeedTostr(int speed);
    void printConfDesc(libusb_device *);
    std::string EPAddTostr(uint8_t add);
    std::string EPAttTostr(uint8_t att);

    void EnableHotPlug();
    static int hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data);
    static int hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data);

    void startSniffing();
    void bulk(libusb_device_handle *devh);

private:
    typedef BaseApplLayer super;

    // NED variables
    bool on;
    bool listUSBdevices;
    bool hotPlug;

    // NED variables
    uint16_t target_vendor_id;
    uint16_t target_product_id;
    int target_interfaceNumber;
    unsigned char target_interruptEP;

    // variables
    TraCI_Commands *TraCI;
    simsignal_t Signal_executeFirstTS;
    simsignal_t Signal_executeEachTS;

    libusb_context *ctx = NULL;  // a libusb session
    static libusb_device_handle *hotPlugHandle;
    libusb_device_handle *dev_handle;
    int EPPacketSize;

    std::map< usb_vender, std::vector<usb_device> > USBids;
    std::map<int, std::string> USBclass = {
            {0, "PER_INTERFACE"},
            {1, "AUDIO"},
            {2, "COMM"},
            {3, "HID"},
            {5, "PHYSICAL"},
            {6, "IMAGING"},
            {7, "PRINTER"},
            {8, "MASS_STORAGE"},
            {9, "HUB"},
            {0xa, "CDC_DATA"},
            {0x0b, "SMART_CARD"},
            {0x0d, "CONTENT_SECURITY"},
            {0x0e, "VIDEO"},
            {0x0f, "PERSONAL_HEALTHCARE"},
            {0x3a, "XBOX"},
            {0xdc, "DIAGNOSTIC_DEVICE"},
            {0xe0, "WIRELESS"},
            {0xef, "MISCELLANEOUS"},
            {0xfe, "APPLICATION_SPEC"},
            {0xff, "VENDOR_SPEC"}
    };

    cMessage* USBevents;
    cMessage* USBInterrupt;
};

}

#endif

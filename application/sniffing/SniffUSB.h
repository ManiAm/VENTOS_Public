/****************************************************************************/
/// @file    SniffUSB.h
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
#include <Appl.h>
#include "TraCI_Extend.h"
#include <libusb-1.0/libusb.h>
#include <unordered_map>

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


class SniffUSB : public BaseApplLayer
{
public:
    virtual ~SniffUSB();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);
    virtual void receiveSignal(cComponent *, simsignal_t, long);

private:
    void executeFirstTimeStep();
    void executeEachTimestep(bool);

    void getUSBidsFromFile();
    std::string USBversion(uint16_t version);
    std::string USBspeed(int speed);
    std::string decodeEPAdd(uint8_t add);
    std::string decodeEPAtt(uint8_t att);
    void getUSBdevices();
    std::vector<std::string> USBidToName(uint16_t, uint16_t);
    void printdev(libusb_device *dev);

    void EnableHotPlug();
    static int hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data);
    static int hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data);

    void startSniffing();

private:
    // NED variables
    bool on;
    bool listUSBdevices;
    bool listUSBdevicesDetailed;
    bool hotPlug;

    // variables
    TraCI_Extend *TraCI;
    simsignal_t Signal_executeFirstTS;
    simsignal_t Signal_executeEachTS;

    static libusb_context *ctx;  // a libusb session
    static libusb_device_handle *hotPlugHandle;

    std::map< usb_vender, std::vector<usb_device> > USBids;
    cMessage* USBevents;
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
};

}

#endif

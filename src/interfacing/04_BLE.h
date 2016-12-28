/****************************************************************************/
/// @file    BLE.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Feb 2016
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

#ifndef SNIFFBLUETOOTHLE_H
#define SNIFFBLUETOOTHLE_H

#include "interfacing/03_Bluetooth.h"

namespace VENTOS {

class BLE : public Bluetooth
{
private:
    typedef Bluetooth super;

    // NED variables
    bool BLE_on;

    boost::filesystem::path cached_LEBT_devices_filePATH;

    // stores all LE BT devices (cached and newly detected)
    std::map<std::string /*BT address*/, BTdevEntry> allLEBTdevices;

public:
    virtual ~BLE();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(omnetpp::cMessage *);

protected:
    void initialize_withTraCI();
    void executeEachTimestep();

private:
    void loadCachedDevices();
    void saveCachedDevices();

    void lescan(int dev_id, uint8_t scan_type, uint16_t interval, uint16_t window, uint8_t own_type, uint8_t filter_policy, int scanTime);

    std::map<std::string /*BT add*/, std::vector<std::string>> print_advertising_devices(int dd, int timeout);
    std::string parse_name(uint8_t* data, size_t size);
    int parse_flags(uint8_t* data, size_t size);
    int parse_appearance(uint8_t* data, size_t size);

    uint16_t leCreateConnection(std::string bdaddr);
};

}

#endif

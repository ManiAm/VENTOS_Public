/****************************************************************************/
/// @file    SniffBluetoothLE.h
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

#ifndef SNIFFBLUETOOTHLE
#define SNIFFBLUETOOTHLE

#include "03_SniffBluetooth.h"

namespace VENTOS {

class SniffBluetoothLE : public SniffBluetooth
{
public:
    virtual ~SniffBluetoothLE();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);

protected:
    void executeFirstTimeStep();
    void executeEachTimestep();

    void cmd_cmd(int dev_id, uint8_t ogf, uint16_t ocf, std::string payload);
    void hex_dump(std::string pref, int width, unsigned char *buf, int len);

private:
    void loadCachedDevices();
    void saveCachedDevices();

    void lescan(int);
    std::map<std::string /*BT add*/, std::vector<std::string>> print_advertising_devices(int dd, int timeout);
    int parse_flags(uint8_t* data, size_t size);
    int parse_appearance(uint8_t* data, size_t size);
    std::string parse_name(uint8_t* data, size_t size);

    uint16_t leCreateConnection(std::string bdaddr);

private:
    // NED variables
    bool LEBTon;

    boost::filesystem::path cached_LEBT_devices_filePATH;

    // stores all LE BT devices (cached and newly detected)
    std::map<std::string /*BT address*/, BTdevEntry> allLEBTdevices;
};

}

#endif

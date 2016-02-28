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

#include <BaseApplLayer.h>
#include "TraCI_Commands.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

// un-defining ev!
// why? http://stackoverflow.com/questions/24103469/cant-include-the-boost-filesystem-header
#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

namespace VENTOS {

class LEBTdevEntry
{
public:
    std::string name;       // LE BT device name
    std::string timeStamp;  // LE BT device last detection time

    LEBTdevEntry(std::string str1, std::string str2)
    {
        this->name = str1;
        this->timeStamp = str2;
    }
};

class SniffBluetoothLE : public BaseApplLayer
{
public:
    virtual ~SniffBluetoothLE();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(cMessage *);
    virtual void receiveSignal(cComponent *, simsignal_t, long);

private:
    void executeFirstTimeStep();
    void executeEachTimestep();

    void loadCachedDevices();
    void lescan();
    void print_advertising_devices(int dd, int timeout);
    const std::string currentDateTime();
    int parse_flags(uint8_t* data, size_t size);
    int parse_appearance(uint8_t* data, size_t size);
    std::string parse_name(uint8_t* data, size_t size);
    void saveCachedDevices();
    void startSniffing();
    void got_packet(const struct pcap_pkthdr *header, const u_char *packet);

private:
    // NED variables
    bool on;

    // variables
    TraCI_Commands *TraCI;
    simsignal_t Signal_executeFirstTS;
    simsignal_t Signal_executeEachTS;
    boost::filesystem::path cached_LEBT_devices_filePATH;

    // stores all LE BT devices (cached and newly detected)
    std::map<std::string /*BT address*/, LEBTdevEntry> allLEBTdevices;
};

}

#endif

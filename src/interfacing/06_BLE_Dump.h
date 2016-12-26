/****************************************************************************/
/// @file    BLE_Dump.h
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

#ifndef SNIFFBLUETOOTHDUMP_H
#define SNIFFBLUETOOTHDUMP_H

#include "05_BLE_Advertisement.h"

namespace VENTOS {

class BLE_Dump : public BLE_Advertisement
{
private:
    typedef BLE_Advertisement super;

    // NED variables
    bool dump_On;
    int BLE_dump_deviceID;

    omnetpp::simsignal_t Signal_initialize_withTraCI;
    omnetpp::simsignal_t Signal_executeEachTS;

public:
    virtual ~BLE_Dump();
    virtual void finish();
    virtual void initialize(int);
    virtual void handleMessage(omnetpp::cMessage *);
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject* details);

protected:
    void initialize_withTraCI();
    void executeEachTimestep();

private:
    void lescanEnable(int dev_id, uint8_t scan_type, uint16_t interval, uint16_t window, uint8_t own_type, uint8_t filter_policy);
    void lescanDisable(int dev_id);
    int open_socket(int dev_id);
    void process_frames(int dev_id, int sock, int timeout);
    void hex_dump(struct frame *frm);
    void hci_dump(struct frame *frm);
};

}

#endif

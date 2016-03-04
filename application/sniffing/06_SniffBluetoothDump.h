/****************************************************************************/
/// @file    SniffBluetoothDump.h
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

#ifndef SNIFFBLUETOOTHDUMP
#define SNIFFBLUETOOTHDUMP

#include "05_AdvertiseBeacon.h"

namespace VENTOS {

class SniffBluetoothDump : public AdvertiseBeacon
{
public:
    virtual ~SniffBluetoothDump();
    virtual void finish();
    virtual void initialize(int);
    virtual int numInitStages() const
    {
        return 2;  // stage 0, 1
    }
    virtual void handleMessage(cMessage *);
    virtual void receiveSignal(cComponent *, simsignal_t, long);

protected:
    void executeFirstTimeStep();
    void executeEachTimestep();

private:
    int open_socket(int dev_id);
    void process_frames(int dev_id, int sock, int timeout);
    void hex_dump(struct frame *frm);

private:
    // NED variables
    bool dump_On;
    int BLE_dump_deviceID;

    simsignal_t Signal_executeFirstTS;
    simsignal_t Signal_executeEachTS;
};

}

#endif

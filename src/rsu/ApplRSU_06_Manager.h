/****************************************************************************/
/// @file    ApplRSU_06_Manager.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    August 2013
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

#ifndef APPLRSUMANAGER_H_
#define APPLRSUMANAGER_H_

#include "ApplRSU_05_CRL.h"

namespace VENTOS {

class ApplRSUManager : public ApplRSUCRL
{
public:
    ~ApplRSUManager();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleLowerMsg(cMessage* msg);
    virtual void handleSelfMsg(cMessage* msg);
    virtual void receiveSignal(cComponent *, simsignal_t, long);
    virtual void receiveSignal(cComponent *, simsignal_t, cObject *);

protected:
    void virtual executeEachTimeStep();

    virtual void onBeaconVehicle(BeaconVehicle*);
    virtual void onBeaconBicycle(BeaconBicycle*);
    virtual void onBeaconPedestrian(BeaconPedestrian*);
    virtual void onBeaconRSU(BeaconRSU*);
    virtual void onData(LaneChangeMsg*);

private:
    typedef ApplRSUCRL super;

    simsignal_t Signal_executeEachTS;
    simsignal_t Signal_beaconSignaling;
    static const simsignalwrap_t mobilityStateChangedSignal;
};

}

#endif

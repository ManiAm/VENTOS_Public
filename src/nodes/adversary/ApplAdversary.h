/****************************************************************************/
/// @file    ApplAdversary.h
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

#ifndef APPLADVERSARY_H_
#define APPLADVERSARY_H_

#include "baseAppl/04_BaseWaveApplLayer.h"
#include "msg/BeaconVehicle_m.h"
#include "msg/DummyMsg_m.h"

namespace VENTOS {

class ApplAdversary : public BaseWaveApplLayer
{
private:
    typedef BaseWaveApplLayer super;

protected:
    // NED variables
    int attackMode = -1;
    double attackStartTime = -1;

    typedef enum attackMode
    {
        attackMode_falsification = 0,
        attackMode_replay,
        attackMode_jamming,

        attackMode_MAX,

    }attackMode_t;

    BaseConnectionManager* cc;
    omnetpp::simsignal_t Signal_executeEachTS;

public:
    ~ApplAdversary();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject *);
    virtual void handleLowerMsg(omnetpp::cMessage* msg);
    virtual void handleSelfMsg(omnetpp::cMessage* msg);

private:
    void DoFalsificationAttack(BeaconVehicle * wsm);
    void DoReplayAttack(BeaconVehicle * wsm);
    void DoJammingAttack();

    DummyMsg* CreateDummyMessage();
};

}

#endif

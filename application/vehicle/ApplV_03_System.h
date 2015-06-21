/****************************************************************************/
/// @file    ApplV_03_System.h
/// @author  Dylan Smith <dilsmith@ucdavis.edu>
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
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

#ifndef ApplVSystem_H
#define ApplVSystem_H

#include "ApplV_02_Beacon.h"
#include "Router.h"
#include "Hypertree.h"

namespace VENTOS {

class Router;
class Hypertree;

class ApplVSystem : public ApplVBeacon
{
public:
    ~ApplVSystem();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);

protected:
    virtual void handleSelfMsg(cMessage*);
    virtual void handlePositionUpdate(cObject*);

protected:
    // NED variables (beaconing parameters)
    bool requestRoutes;         //like sendBeacons;
    double requestInterval;     //like beaconInterval;
    double maxOffsetSystem;     //From beacon maxOffset
    int systemMsgLengthBits;
    int systemMsgPriority;      //like beaconPriority
    RouterMessage routingMode;
    double hypertreeUpdateInterval;
    bool requestReroutes;
    int numReroutes;

    Router* router;
    void reroute();

    // Class variables
    cMessage* sendSystemMsgEvt;
    simsignal_t Signal_router;
    simsignal_t Signal_system;

    // Routing
    std::string targetNode;
    Hypertree* ht;
};

}

#endif


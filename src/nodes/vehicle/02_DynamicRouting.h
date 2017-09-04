/****************************************************************************/
/// @file    DynamicRouting.h
/// @author  Dylan Smith <dilsmith@ucdavis.edu>
/// @author  Huajun Chai <hjchai@ucdavis.edu>
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

#ifndef ApplVDYNAMICROUTING_H
#define ApplVDYNAMICROUTING_H

#include "nodes/vehicle/01_Beacon.h"
#include "router/Router.h"
#include "router/Hypertree.h"

namespace VENTOS {

class Router;
class Hypertree;

class ApplVDynamicRouting : public ApplVBeacon
{
private:
    typedef ApplVBeacon super;

protected:
    // NED variables (beaconing parameters)
    bool requestRoutes;         //like sendBeacons;
    int debugLevel;
    double requestInterval;     //like beaconInterval;
    int systemMsgLengthBits;
    int systemMsgPriority;      //like beaconPriority
    RouterMessage routingMode;
    double hypertreeUpdateInterval;
    bool requestReroutes;
    int numReroutes;

    Router* router;

    // Class variables
    omnetpp::cMessage* sendSystemMsgEvt = NULL;

    omnetpp::simsignal_t Signal_router = -1;
    omnetpp::simsignal_t Signal_system = -1;

    // Routing
    std::string targetNode;
    Hypertree* ht;

public:
    ~ApplVDynamicRouting();
    virtual void initialize(int stage);
    virtual void finish();
    virtual void receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, omnetpp::cObject *obj, cObject* details);

protected:
    virtual void handleSelfMsg(omnetpp::cMessage*);
    virtual void handlePositionUpdate(cObject*);
    void reroute();
};

}

#endif


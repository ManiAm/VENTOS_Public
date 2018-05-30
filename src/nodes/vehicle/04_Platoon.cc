/****************************************************************************/
/// @file    04_Platoon.cc
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

#include "nodes/vehicle/04_Platoon.h"

namespace VENTOS {

Define_Module(VENTOS::ApplVPlatoon);

ApplVPlatoon::~ApplVPlatoon()
{
    cancelAndDelete(platoonMonitorTIMER);
}


void ApplVPlatoon::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        plnMode = par("plnMode").intValue();
        if(plnMode != platoonNoManagement && plnMode != platoonManagement)
            return;

        myPlnID = par("myPlnID").stringValue();
        if(myPlnID == "")
            throw omnetpp::cRuntimeError("pltID is empty in vehicle '%s'", SUMOID.c_str());

        myPlnDepth = par("myPlnDepth").intValue();
        if(myPlnDepth < 0)
            throw omnetpp::cRuntimeError("pltDepth is invalid in vehicle '%s'", SUMOID.c_str());

        // I am the platoon leader
        if(myPlnDepth == 0)
        {
            // plnSize is known in platoon leader only
            plnSize = par("plnSize").intValue();
            if(plnSize <= 0)
                throw omnetpp::cRuntimeError("pltSize is invalid in vehicle '%s'", SUMOID.c_str());

            for(int i = 0; i < plnSize; i++)
            {
                std::string vehID = "";
                if(i == 0)
                    vehID = myPlnID;
                else
                    vehID = myPlnID + "." + std::to_string(i);
                plnMembersList.push_back(vehID);
            }

            // platoon leader registers the platoon with SUMO
            TraCI->vehiclePlatoonInit(SUMOID, plnMembersList);
        }

        record_platoon_stat = par("record_platoon_stat").boolValue();
        if(record_platoon_stat)
        {
            platoonMonitorTIMER = new omnetpp::cMessage("platoon_monitor");
            scheduleAt(omnetpp::simTime(), platoonMonitorTIMER);
        }

        updateInterval = (double)TraCI->simulationGetDelta() / 1000.;

        WATCH(plnMode);
        WATCH(myPlnID);
        WATCH(myPlnDepth);
        WATCH(plnSize);
    }
}


void ApplVPlatoon::finish()
{
    super::finish();
}


void ApplVPlatoon::handleSelfMsg(omnetpp::cMessage* msg)
{
    // pass it down!
    super::handleSelfMsg(msg);
}


void ApplVPlatoon::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down!
    super::onBeaconVehicle(wsm);
}


void ApplVPlatoon::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down!
    super::onBeaconRSU(wsm);
}


// is called, every time the position of vehicle changes
void ApplVPlatoon::handlePositionUpdate(cObject* obj)
{
    // pass it down!
    super::handlePositionUpdate(obj);
}

}

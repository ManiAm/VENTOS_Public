/****************************************************************************/
/// @file    ApplV_06_Entry.cc
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

#include "vehicle/platoon/02_PlatoonMg.h"

namespace VENTOS {

void ApplVPlatoonMg::entry_handleSelfMsg(omnetpp::cMessage* msg)
{
    if(!entryEnabled)
        return;

    // start the Entry maneuver
    if(msg == entryManeuverEvt && vehicleState == state_idle)
    {
        // check if we are at lane 0
        if(TraCI->vehicleGetLaneIndex(SUMOID) == 0)
        {
            TraCI->vehicleSetClass(SUMOID, "vip");   // change vClass to vip

            int32_t bitset = TraCI->vehicleBuildLaneChangeMode(10, 01, 01, 01, 01);
            TraCI->vehicleSetLaneChangeMode(SUMOID, bitset);   // alter 'lane change' mode
            TraCI->vehicleChangeLane(SUMOID, 1, 5);   // change to lane 1 (special lane)

            // change state to waitForLaneChange
            vehicleState = state_waitForLaneChange;
            reportStateToStat();

            scheduleAt(omnetpp::simTime() + 0.1, plnTIMER0);
        }
    }
    else if(msg == plnTIMER0 && vehicleState == state_waitForLaneChange)
    {
        // check if we change lane
        if(TraCI->vehicleGetLaneIndex(SUMOID) == 1)
        {
            // make it a free agent
            plnID = SUMOID;
            myPlnDepth = 0;
            plnSize = 1;
            plnMembersList.push_back(SUMOID);
            TraCI->vehicleSetTimeGap(SUMOID, TP);

            busy = false;

            // get my leading vehicle
            std::vector<std::string> vleaderIDnew = TraCI->vehicleGetLeader(SUMOID, sonarDist);
            std::string vleaderID = vleaderIDnew[0];

            // if no leading, set speed to 5 m/s
            if(vleaderID == "")
            {
                TraCI->vehicleSetSpeed(SUMOID, 5.);
            }
            // if I have leading, set speed to max
            else
            {
                TraCI->vehicleSetSpeed(SUMOID, 30.);
            }

            // change color to red!
            RGB newColor = Color::colorNameToRGB("red");
            TraCI->vehicleSetColor(SUMOID, newColor);

            // change state to platoon leader
            vehicleState = state_platoonLeader;
            reportStateToStat();
        }
        else
            scheduleAt(omnetpp::simTime() + 0.1, plnTIMER0);
    }
}


void ApplVPlatoonMg::entry_BeaconFSM(BeaconVehicle* wsm)
{

}


void ApplVPlatoonMg::entry_DataFSM(PlatoonMsg *wsm)
{

}

}

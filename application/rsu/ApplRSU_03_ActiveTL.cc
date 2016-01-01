/****************************************************************************/
/// @file    ApplRSU_03_ActiveTL.cc
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

#include "ApplRSU_03_ActiveTL.h"

namespace VENTOS {

Define_Module(VENTOS::ApplRSUTLVANET);

ApplRSUTLVANET::~ApplRSUTLVANET()
{

}


void ApplRSUTLVANET::initialize(int stage)
{
    ApplRSUMonitor::initialize(stage);

    if (stage==0)
    {
        if (!activeDetection)
            return;

        if(!monitorVehApproach)
            error("monitorVehApproach flag is not set!");

        // we need this RSU to be associated with a TL
        if(myTLid == "")
            error("The id of %s does not match with any TL. Check RSUsLocation.xml file!", myFullId);

        // for each incoming lane
        for(auto &it2 : lanesTL)
        {
            std::string lane = it2.first;

            // get the max speed on this lane
            double maxV = TraCI->laneGetMaxSpeed(lane);

            // calculate initial passageTime for this lane
            // todo: change fix value
            double pass = 35. / maxV;

            // check if not greater than Gmin
            if(pass > minGreenTime)
            {
                std::cout << "WARNING (" << myFullId << "): Passage time is greater than Gmin in lane " << lane << endl;
                pass = minGreenTime;
            }

            // add this lane to the laneInfo map
            laneInfoEntry *entry = new laneInfoEntry(myTLid, 0, 0, pass, 0, std::map<std::string, queuedVehiclesEntry>());
            laneInfo.insert( std::make_pair(lane, *entry) );
        }
    }
}


void ApplRSUTLVANET::finish()
{
    ApplRSUMonitor::finish();
}


void ApplRSUTLVANET::handleSelfMsg(cMessage* msg)
{
    ApplRSUMonitor::handleSelfMsg(msg);
}


void ApplRSUTLVANET::executeEachTimeStep(bool simulationDone)
{
    ApplRSUMonitor::executeEachTimeStep(simulationDone);
}


void ApplRSUTLVANET::onBeaconVehicle(BeaconVehicle* wsm)
{
    ApplRSUMonitor::onBeaconVehicle(wsm);
}


void ApplRSUTLVANET::onBeaconBicycle(BeaconBicycle* wsm)
{
    ApplRSUMonitor::onBeaconBicycle(wsm);
}


void ApplRSUTLVANET::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    ApplRSUMonitor::onBeaconPedestrian(wsm);
}


void ApplRSUTLVANET::onBeaconRSU(BeaconRSU* wsm)
{
    ApplRSUMonitor::onBeaconRSU(wsm);
}


void ApplRSUTLVANET::onData(LaneChangeMsg* wsm)
{
    ApplRSUMonitor::onData(wsm);
}


// update laneInfo
void ApplRSUTLVANET::UpdateLaneInfoAdd(std::string lane, std::string sender, std::string senderType, double speed)
{
    // check again to make sure activeDetection is on
    if (!activeDetection)
        return;

    // update laneInfo map
    std::map<std::string, laneInfoEntry>::iterator loc = laneInfo.find(lane);

    if(loc != laneInfo.end())
    {
        // update total vehicle count
        loc->second.totalVehCount = loc->second.totalVehCount + 1;

        // this is the first vehicle on this lane
        if(loc->second.totalVehCount == 1)
            loc->second.firstDetectedTime = simTime().dbl();

        // update detectedTime
        loc->second.lastDetectedTime = simTime().dbl();

        // add it as a queued vehicle on this lane
        queuedVehiclesEntry *newVeh = new queuedVehiclesEntry( senderType, simTime().dbl(), speed );
        loc->second.queuedVehicles.insert( std::make_pair(sender, *newVeh) );
    }
    else
        error("lane %s does not exist in laneInfo map!", lane.c_str());

    // get the approach speed from the beacon
    double approachSpeed = speed;
    // update passage time for this lane
    if(approachSpeed > 0)
    {
        // calculate passageTime for this lane
        // todo: change fix value
        double pass = 35. / approachSpeed;
        // check if not greater than Gmin
        if(pass > minGreenTime)
            pass = minGreenTime;

        // update passage time
        std::map<std::string, laneInfoEntry>::iterator location = laneInfo.find(lane);
        location->second.passageTime = pass;
    }
}


// update laneInfo
void ApplRSUTLVANET::UpdateLaneInfoRemove(std::string lane, std::string sender)
{
    // check again to make sure activeDetection is on
    if (!activeDetection)
        return;

    std::map<std::string, laneInfoEntry>::iterator loc = laneInfo.find(lane);

    if(loc != laneInfo.end())
    {
        // remove it from the queued vehicles
        std::map<std::string, queuedVehiclesEntry>::iterator ref = loc->second.queuedVehicles.find(sender);
        if(ref != loc->second.queuedVehicles.end())
            loc->second.queuedVehicles.erase(ref);
        else
            error("vehicle %s was not added into lane %s in laneInfo map!", sender.c_str(), lane.c_str());
    }
    else
        error("lane %s does not exist in laneInfo map!", lane.c_str());
}

}

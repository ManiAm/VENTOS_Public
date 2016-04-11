/****************************************************************************/
/// @file    ApplRSU_02_Monitor.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Dec 2015
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

#include "ApplRSU_02_Monitor.h"
#include <algorithm>
#include <iomanip>

// un-defining ev!
// why? http://stackoverflow.com/questions/24103469/cant-include-the-boost-filesystem-header
#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

namespace VENTOS {

std::vector<detectedVehicleEntry> ApplRSUMonitor::Vec_detectedVehicles;

Define_Module(VENTOS::ApplRSUMonitor);

ApplRSUMonitor::~ApplRSUMonitor()
{

}


void ApplRSUMonitor::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        // note that TLs set 'activeDetection' after RSU creation
        activeDetection = par("activeDetection").boolValue();
        collectVehApproach = par("collectVehApproach").boolValue();

        // get a list of all TLs
        std::list<std::string> TLlist = TraCI->TLGetIDList();

        // look for myTLid in TL list
        auto it = std::find(TLlist.begin(), TLlist.end(), myTLid);

        if(it != TLlist.end())
        {
            // for each incoming lane in this TL
            std::list<std::string> lan = TraCI->TLGetControlledLanes(myTLid);

            // remove duplicate entries
            lan.unique();

            // for each incoming lane
            for(auto &it2 :lan)
            {
                std::string lane = it2;

                lanesTL[lane] = myTLid;

                // get the max speed on this lane
                double maxV = TraCI->laneGetMaxSpeed(lane);

                // calculate initial passageTime for this lane
                // todo: change fix value
                double pass = 35. / maxV;

                // check if not greater than Gmin
                if(pass > minGreenTime)
                {
                    std::cout << std::endl;
                    std::cout << "WARNING (" << myFullId << "): Passage time is greater than Gmin in lane " << lane << endl;
                    pass = minGreenTime;
                }

                // add this lane to the laneInfo map
                laneInfoEntry *entry = new laneInfoEntry(myTLid, 0, 0, pass, 0, std::map<std::string, allVehiclesEntry>());
                laneInfo.insert( std::make_pair(lane, *entry) );
            }
        }
        else if(!TLlist.empty())
        {
            printf("%s's name (%s) does not match with any of %lu TLs \n", myFullId, SUMOID.c_str(), TLlist.size());
            std::cout.flush();
        }
    }
}


void ApplRSUMonitor::finish()
{
    super::finish();

    // write to file at the end of simulation
    if(activeDetection && collectVehApproach)
        saveVehApproach();
}


void ApplRSUMonitor::handleSelfMsg(cMessage* msg)
{
    super::handleSelfMsg(msg);
}


void ApplRSUMonitor::executeEachTimeStep()
{
    super::executeEachTimeStep();
}


void ApplRSUMonitor::onBeaconVehicle(BeaconVehicle* wsm)
{
    activeDetection = par("activeDetection").boolValue();
    if(activeDetection)
        onBeaconAny(wsm);
}


void ApplRSUMonitor::onBeaconBicycle(BeaconBicycle* wsm)
{
    activeDetection = par("activeDetection").boolValue();
    if(activeDetection)
        onBeaconAny(wsm);
}


void ApplRSUMonitor::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    activeDetection = par("activeDetection").boolValue();
    if(activeDetection)
        onBeaconAny(wsm);
}


void ApplRSUMonitor::onBeaconRSU(BeaconRSU* wsm)
{

}


void ApplRSUMonitor::onData(LaneChangeMsg* wsm)
{

}


// update variables upon reception of any beacon (vehicle, bike, pedestrian)
template <typename T> void ApplRSUMonitor::onBeaconAny(T wsm)
{
    std::string sender = wsm->getSender();
    Coord pos = wsm->getPos();

    // todo: change from fix values
    // Check if entity is in the detection region:
    // Coordinates can be locations of the middle of the LD ( 851.1 <= x <= 948.8 and 851.1 <= y <= 948.8)
    if ( (pos.x >= 486.21) && (pos.x <= 1313.91) && (pos.y >= 486.21) && (pos.y <= 1313.85) )
    {
        std::string lane = wsm->getLane();

        // If on one of the incoming lanes
        if(lanesTL.find(lane) != lanesTL.end() && lanesTL[lane] == myTLid)
        {
            // search queue for this vehicle
            const detectedVehicleEntry *searchFor = new detectedVehicleEntry(sender);
            std::vector<detectedVehicleEntry>::iterator counter = std::find(Vec_detectedVehicles.begin(), Vec_detectedVehicles.end(), *searchFor);

            // we have already added this vehicle
            if (counter != Vec_detectedVehicles.end() && counter->leaveTime == -1)
            {
                LaneInfoUpdate(lane, sender, wsm->getSenderType(), wsm->getSpeed());

                return;
            }
            // the vehicle is visiting the intersection more than once
            else if (counter != Vec_detectedVehicles.end() && counter->leaveTime != -1)
            {
                counter->entryTime = simTime().dbl();
                counter->entrySpeed = wsm->getSpeed();
                counter->pos = wsm->getPos();
                counter->leaveTime = -1;
            }
            else
            {
                // Add entry
                detectedVehicleEntry *tmp = new detectedVehicleEntry(sender, wsm->getSenderType(), lane, wsm->getPos(), myTLid, simTime().dbl(), -1, wsm->getSpeed());
                Vec_detectedVehicles.push_back(*tmp);
            }

            LaneInfoAdd(lane, sender, wsm->getSenderType(), wsm->getSpeed());
        }
        // Else exiting queue area, so log the leave time
        else
        {
            // search queue for this vehicle
            const detectedVehicleEntry *searchFor = new detectedVehicleEntry(sender);
            std::vector<detectedVehicleEntry>::iterator counter = std::find(Vec_detectedVehicles.begin(), Vec_detectedVehicles.end(), *searchFor);

            if (counter == Vec_detectedVehicles.end())
                error("vehicle %s does not exist in the queue!", sender.c_str());

            if(counter->leaveTime == -1)
            {
                counter->leaveTime = simTime().dbl();

                LaneInfoRemove(counter->lane, sender);
            }
        }
    }
}


void ApplRSUMonitor::saveVehApproach()
{
    boost::filesystem::path filePath;

    if(ev.isGUI())
    {
        filePath = "results/gui/vehApproach.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << std::setfill('0') << std::setw(3) << currentRun << "_vehApproach.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write simulation parameters at the beginning of the file in CMD mode
    if(!ev.isGUI())
    {
        // get the current config name
        std::string configName = ev.getConfigEx()->getVariable("configname");

        // get number of total runs in this config
        int totalRun = ev.getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();

        // get all iteration variables
        std::vector<std::string> iterVar = ev.getConfigEx()->unrollConfig(configName.c_str(), false);

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n\n\n", iterVar[currentRun].c_str());
    }

    // write header
    fprintf (filePtr, "%-20s","vehicleName");
    fprintf (filePtr, "%-20s","vehicleType");
    fprintf (filePtr, "%-15s","lane");
    fprintf (filePtr, "%-15s","posX");
    fprintf (filePtr, "%-15s","posY");
    fprintf (filePtr, "%-15s","TLid");
    fprintf (filePtr, "%-15s","entryTime");
    fprintf (filePtr, "%-15s","entrySpeed");
    fprintf (filePtr, "%-15s\n\n","leaveTime");

    // write body
    for(std::vector<detectedVehicleEntry>::iterator y = Vec_detectedVehicles.begin(); y != Vec_detectedVehicles.end(); ++y)
    {
        fprintf (filePtr, "%-20s ", (*y).vehicleName.c_str());
        fprintf (filePtr, "%-20s ", (*y).vehicleType.c_str());
        fprintf (filePtr, "%-13s ", (*y).lane.c_str());
        fprintf (filePtr, "%-15.2f ", (*y).pos.x);
        fprintf (filePtr, "%-15.2f ", (*y).pos.y);
        fprintf (filePtr, "%-15s ", (*y).TLid.c_str());
        fprintf (filePtr, "%-15.2f ", (*y).entryTime);
        fprintf (filePtr, "%-15.2f", (*y).entrySpeed);
        fprintf (filePtr, "%-15.2f\n", (*y).leaveTime);
    }

    fclose(filePtr);
}


// add a new vehicle
void ApplRSUMonitor::LaneInfoAdd(std::string lane, std::string sender, std::string senderType, double speed)
{
    // look for this lane in laneInfo map
    std::map<std::string, laneInfoEntry>::iterator loc = laneInfo.find(lane);
    if(loc == laneInfo.end())
        error("lane %s does not exist in laneInfo map!", lane.c_str());

    // update total vehicle count
    loc->second.totalVehCount = loc->second.totalVehCount + 1;

    // this is the first vehicle on this lane
    if(loc->second.totalVehCount == 1)
        loc->second.firstDetectedTime = simTime().dbl();

    // update detectedTime
    loc->second.lastDetectedTime = simTime().dbl();

    // get stopping speed threshold
    double stoppingDelayThreshold = 0;
    if(senderType == "bicycle")
        stoppingDelayThreshold = TLptr->par("bikeStoppingDelayThreshold").doubleValue();
    else
        stoppingDelayThreshold = TLptr->par("vehStoppingDelayThreshold").doubleValue();

    // get vehicle status
    int vehStatus = speed > stoppingDelayThreshold ? VEH_STATUS_Driving : VEH_STATUS_Waiting;

    // add it to vehicle list on this lane
    allVehiclesEntry *newVeh = new allVehiclesEntry( senderType, vehStatus, simTime().dbl(), speed );
    loc->second.allVehicles.insert( std::make_pair(sender, *newVeh) );

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


// update vehStatus of vehicles in laneInfo
void ApplRSUMonitor::LaneInfoUpdate(std::string lane, std::string sender, std::string senderType, double speed)
{
    // look for this lane in laneInfo map
    std::map<std::string, laneInfoEntry>::iterator loc = laneInfo.find(lane);
    if(loc == laneInfo.end())
        error("lane %s does not exist in laneInfo map!", lane.c_str());

    // look for this vehicle in this lane
    std::map<std::string, allVehiclesEntry>::iterator ref = loc->second.allVehicles.find(sender);
    if(ref == loc->second.allVehicles.end())
        error("vehicle %s was not added into lane %s in laneInfo map!", sender.c_str(), lane.c_str());


    // get stopping speed threshold
    double stoppingDelayThreshold = 0;
    if(senderType == "bicycle")
        stoppingDelayThreshold = TLptr->par("bikeStoppingDelayThreshold").doubleValue();
    else
        stoppingDelayThreshold = TLptr->par("vehStoppingDelayThreshold").doubleValue();

    // get vehicle status
    int vehStatus = speed > stoppingDelayThreshold ? VEH_STATUS_Driving : VEH_STATUS_Waiting;

    ref->second.vehStatus = vehStatus;
}


// removing an existing vehicle
void ApplRSUMonitor::LaneInfoRemove(std::string lane, std::string sender)
{
    // look for this lane in laneInfo map
    std::map<std::string, laneInfoEntry>::iterator loc = laneInfo.find(lane);
    if(loc == laneInfo.end())
        error("lane %s does not exist in laneInfo map!", lane.c_str());

    // look for this vehicle in this lane
    std::map<std::string, allVehiclesEntry>::iterator ref = loc->second.allVehicles.find(sender);
    if(ref == loc->second.allVehicles.end())
        error("vehicle %s was not added into lane %s in laneInfo map!", sender.c_str(), lane.c_str());

    // remove it from the vehicles list
    loc->second.allVehicles.erase(ref);
}

}

/****************************************************************************/
/// @file    Monitor.cc
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

#include <algorithm>
#include <iomanip>

#undef ev
#include "boost/filesystem.hpp"

#include "nodes/rsu/02_Monitor.h"

namespace VENTOS {

std::vector<std::string> ApplRSUMonitor::junctionList;

Define_Module(VENTOS::ApplRSUMonitor);

ApplRSUMonitor::~ApplRSUMonitor()
{

}


void ApplRSUMonitor::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        record_vehApproach_stat = par("record_vehApproach_stat").boolValue();

        if(!record_vehApproach_stat)
            return;

        check_RSU_pos();
        initVariables();
    }
}


void ApplRSUMonitor::finish()
{
    super::finish();

    // each RSU prints its own version of vehApproach
    save_VehApproach_toFile();
    save_VehApproachPerLane_toFile();
}


void ApplRSUMonitor::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


void ApplRSUMonitor::executeEachTimeStep()
{
    super::executeEachTimeStep();
}


void ApplRSUMonitor::onBeaconVehicle(BeaconVehicle* wsm)
{
    onBeaconAny(wsm);
}


void ApplRSUMonitor::onBeaconBicycle(BeaconBicycle* wsm)
{
    onBeaconAny(wsm);
}


void ApplRSUMonitor::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    onBeaconAny(wsm);
}


void ApplRSUMonitor::onBeaconRSU(BeaconRSU* wsm)
{

}


void ApplRSUMonitor::check_RSU_pos()
{
    if(junctionList.empty())
        junctionList = TraCI->junctionGetIDList();

    if(junctionList.empty())
        throw omnetpp::cRuntimeError("there is no junctions in this network!");

    // the RSU is associated with a TL
    if(myTLid != "")
    {
        // look for the junction with the same name
        auto junc = std::find(junctionList.begin(), junctionList.end(), myTLid);

        if(junc != junctionList.end())
        {
            // calculate the distance from this RSU to the center of the junction
            auto coord = TraCI->junctionGetPosition(myTLid);
            double dist = sqrt(pow(rsu_pos.x - coord.x, 2.) + pow(rsu_pos.y - coord.y, 2.));

            if(dist > 0)
                LOG_WARNING << boost::format("\nWARNING: RSU '%s' is not aligned with the center of intersection. \n") % SUMOID;
        }
    }
    else
    {
        double shortest_dist = std::numeric_limits<int>::max();
        std::string nearest_jun = "";

        // iterate over junctions
        for(auto &junc : junctionList)
        {
            // skip the internal junctions
            if(junc[0] == ':')
                continue;

            // calculate the distance from this RSU to the center of the junction
            auto coord = TraCI->junctionGetPosition(junc);
            double dist = sqrt(pow(rsu_pos.x - coord.x, 2.) + pow(rsu_pos.y - coord.y, 2.));

            if(dist < shortest_dist)
            {
                shortest_dist = dist;
                nearest_jun = junc;
            }
        }

        throw omnetpp::cRuntimeError("RSU '%s' is in the vicinity of intersection '%s'. Did you forget to associate?", SUMOID.c_str(), nearest_jun.c_str());
    }
}


void ApplRSUMonitor::initVariables()
{
    // make sure that this RSU is associated with a TL
    ASSERT(myTLid != "");

    // for each incoming lane in this TL
    auto lanes = TraCI->TLGetControlledLanes(myTLid);

    // remove duplicate entries
    sort( lanes.begin(), lanes.end() );
    lanes.erase( unique( lanes.begin(), lanes.end() ), lanes.end() );

    // for each incoming lane
    for(auto &lane :lanes)
    {
        lanesTL[lane] = myTLid;

        // get the max speed on this lane
        double maxV = TraCI->laneGetMaxSpeed(lane);

        // calculate initial passageTime for this lane -- passage time is used in actuated TSC
        // todo: change fix value
        double pass = 35. / maxV;

        // check if not greater than Gmin
        if(pass > minGreenTime)
        {
            LOG_WARNING << boost::format("\nWARNING: Passage time in lane %1% which is controlled by %2% is greater than Gmin \n") % lane % myFullId << std::flush;

            pass = minGreenTime;
        }

        // add this lane to the laneInfo map
        laneInfoEntry_t entry = {};
        entry.TLid = myTLid;
        entry.passageTime = pass;
        laneInfo.insert( std::make_pair(lane, entry) );
    }
}


// update variables upon reception of any beacon (vehicle, bike, pedestrian)
template <typename T> void ApplRSUMonitor::onBeaconAny(T wsm)
{
    if(!record_vehApproach_stat)
        return;

    TraCICoord pos = wsm->getPos();

    // todo: change from fix values
    // check if entity is in the detection region:
    // coordinates can be locations of the middle of the LD ( 851.1 <= x <= 948.8 and 851.1 <= y <= 948.8)
    if ( (pos.x >= 486.21) && (pos.x <= 1313.91) && (pos.y >= 486.21) && (pos.y <= 1313.85) )
    {
        std::string sender = wsm->getSender();
        std::string lane = wsm->getLane();

        // If on one of the incoming lanes
        if(lanesTL.find(lane) != lanesTL.end() && lanesTL[lane] == myTLid)
        {
            // search queue for this vehicle
            auto counter = std::find_if(vehApproach.begin(), vehApproach.end(), [sender](vehApproachEntry_t const& n)
                    {return n.vehicleName == sender;});

            if(counter == vehApproach.end())
            {
                // Add entry
                vehApproachEntry_t tmp = {};

                tmp.TLid = myTLid;
                tmp.vehicleName = sender;
                tmp.vehicleType = wsm->getSenderType();
                tmp.entryLane = lane;
                tmp.entryPos = wsm->getPos();
                tmp.entrySpeed = wsm->getSpeed();
                tmp.entryTime = omnetpp::simTime().dbl();
                tmp.leaveTime = -1;

                vehApproach.push_back(tmp);

                LaneInfoAdd(lane, sender, wsm->getSenderType(), wsm->getSpeed());
            }
            else
            {
                // the vehicle is still in the intersection
                if (counter->leaveTime == -1)
                    LaneInfoUpdate(lane, sender, wsm->getSpeed());
                // the vehicle is visiting the intersection more than once
                // discard the old visit and record the current one
                else if (counter->leaveTime != -1)
                {
                    counter->entryLane = lane;
                    counter->entryPos = wsm->getPos();
                    counter->entrySpeed = wsm->getSpeed();
                    counter->entryTime = omnetpp::simTime().dbl();
                    counter->leaveTime = -1;

                    LaneInfoAdd(lane, sender, wsm->getSenderType(), wsm->getSpeed());
                }
            }
        }
        // vehicle is exiting the queue area --log the leave time
        else
        {
            // search queue for this vehicle
            auto counter = std::find_if(vehApproach.begin(), vehApproach.end(), [sender](vehApproachEntry_t const& n)
                    {return n.vehicleName == sender;});

            if (counter == vehApproach.end())
                throw omnetpp::cRuntimeError("vehicle %s does not exist in the queue!", sender.c_str());

            if(counter->leaveTime == -1)
            {
                counter->leaveTime = omnetpp::simTime().dbl();
                LaneInfoRemove(counter->entryLane, sender);
            }
        }
    }
}


// add a new vehicle
void ApplRSUMonitor::LaneInfoAdd(std::string lane, std::string sender, std::string senderType, double speed)
{
    // look for this lane in laneInfo map
    auto loc = laneInfo.find(lane);
    if(loc == laneInfo.end())
        throw omnetpp::cRuntimeError("lane %s does not exist in laneInfo map!", lane.c_str());

    // update total vehicle count
    loc->second.totalVehCount = loc->second.totalVehCount + 1;

    // this is the first vehicle on this lane
    if(loc->second.totalVehCount == 1)
        loc->second.firstDetectedTime = omnetpp::simTime().dbl();

    // update detectedTime
    loc->second.lastDetectedTime = omnetpp::simTime().dbl();

    // add it to vehicle list on this lane
    vehicleEntryRSU_t newVeh = {};

    newVeh.vehType = senderType;
    newVeh.entryTime = omnetpp::simTime().dbl();
    newVeh.entrySpeed = speed;
    newVeh.currentSpeed = speed;

    loc->second.allVehicles.insert( std::make_pair(sender, newVeh) );

    // update passage time for this lane
    if(speed > 0)
    {
        // calculate passageTime for this lane
        // todo: change fix value
        double pass = 35. / speed;
        // check if not greater than Gmin
        if(pass > minGreenTime)
            pass = minGreenTime;

        // update passage time
        auto location = laneInfo.find(lane);
        location->second.passageTime = pass;
    }

    vehApproachPerLaneEntry_t tmp = {};
    tmp.time = omnetpp::simTime().dbl();
    tmp.laneInfo = laneInfo;

    vehApproachPerLane.push_back(tmp);
}


// update currentSpeed of vehicles in laneInfo
void ApplRSUMonitor::LaneInfoUpdate(std::string lane, std::string sender, double speed)
{
    // look for this lane in laneInfo map
    auto loc = laneInfo.find(lane);
    if(loc == laneInfo.end())
        throw omnetpp::cRuntimeError("lane %s does not exist in laneInfo map!", lane.c_str());

    // look for this vehicle in this lane
    auto ref = loc->second.allVehicles.find(sender);
    if(ref == loc->second.allVehicles.end())
        throw omnetpp::cRuntimeError("vehicle %s was not added into lane %s in laneInfo map!", sender.c_str(), lane.c_str());

    ref->second.currentSpeed = speed;
}


// removing an existing vehicle
void ApplRSUMonitor::LaneInfoRemove(std::string lane, std::string sender)
{
    // look for this lane in laneInfo map
    auto loc = laneInfo.find(lane);
    if(loc == laneInfo.end())
        throw omnetpp::cRuntimeError("lane %s does not exist in laneInfo map!", lane.c_str());

    // look for this vehicle in this lane
    auto ref = loc->second.allVehicles.find(sender);
    if(ref == loc->second.allVehicles.end())
        throw omnetpp::cRuntimeError("vehicle %s was not added into lane %s in laneInfo map!", sender.c_str(), lane.c_str());

    // remove it from the vehicles list
    loc->second.allVehicles.erase(ref);

    vehApproachPerLaneEntry_t tmp = {};
    tmp.time = omnetpp::simTime().dbl();
    tmp.laneInfo = laneInfo;

    vehApproachPerLane.push_back(tmp);
}


void ApplRSUMonitor::save_VehApproach_toFile()
{
    if(vehApproach.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_vehApproach_%s.txt") % currentRun % myFullId;

    boost::filesystem::path filePath ("results");
    filePath /= fileName.str();

    FILE *filePtr = fopen (filePath.c_str(), "w");
    if (!filePtr)
        throw omnetpp::cRuntimeError("Cannot create file '%s'", filePath.c_str());

    // write simulation parameters at the beginning of the file
    {
        // get the current config name
        std::string configName = omnetpp::getEnvir()->getConfigEx()->getVariable("configname");

        std::string iniFile = omnetpp::getEnvir()->getConfigEx()->getVariable("inifile");

        // PID of the simulation process
        std::string processid = omnetpp::getEnvir()->getConfigEx()->getVariable("processid");

        // globally unique identifier for the run, produced by
        // concatenating the configuration name, run number, date/time, etc.
        std::string runID = omnetpp::getEnvir()->getConfigEx()->getVariable("runid");

        // get number of total runs in this config
        int totalRun = omnetpp::getEnvir()->getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

        // get configuration name
        std::vector<std::string> iterVar = omnetpp::getEnvir()->getConfigEx()->getConfigChain(configName.c_str());

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "iniFile         %s\n", iniFile.c_str());
        fprintf (filePtr, "processID       %s\n", processid.c_str());
        fprintf (filePtr, "runID           %s\n", runID.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n", iterVar[0].c_str());
        fprintf (filePtr, "sim timeStep    %u ms\n", TraCI->simulationGetDelta());
        fprintf (filePtr, "startDateTime   %s\n", TraCI->simulationGetStartTime_str().c_str());
        fprintf (filePtr, "endDateTime     %s\n", TraCI->simulationGetEndTime_str().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", TraCI->simulationGetDuration_str().c_str());
    }

    // write header
    fprintf (filePtr, "%-25s","vehicleName");
    fprintf (filePtr, "%-20s","vehicleType");
    fprintf (filePtr, "%-10s","TLid");
    fprintf (filePtr, "%-15s","entryLane");
    fprintf (filePtr, "%-15s","entryPosX");
    fprintf (filePtr, "%-15s","entryPosY");
    fprintf (filePtr, "%-15s","entrySpeed");
    fprintf (filePtr, "%-15s","entryTime");
    fprintf (filePtr, "%-15s\n\n","leaveTime");

    // write body
    for(auto &y : vehApproach)
    {
        fprintf (filePtr, "%-25s", y.vehicleName.c_str());
        fprintf (filePtr, "%-20s", y.vehicleType.c_str());
        fprintf (filePtr, "%-10s", y.TLid.c_str());
        fprintf (filePtr, "%-15s", y.entryLane.c_str());
        fprintf (filePtr, "%-15.2f", y.entryPos.x);
        fprintf (filePtr, "%-15.2f", y.entryPos.y);
        fprintf (filePtr, "%-15.2f", y.entrySpeed);
        fprintf (filePtr, "%-15.2f", y.entryTime);
        fprintf (filePtr, "%-15.2f\n", y.leaveTime);
    }

    fclose(filePtr);
}


void ApplRSUMonitor::save_VehApproachPerLane_toFile()
{
    if(vehApproachPerLane.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_vehApproachPerLane_%s.txt") % currentRun % myFullId;

    boost::filesystem::path filePath ("results");
    filePath /= fileName.str();

    FILE *filePtr = fopen (filePath.c_str(), "w");
    if (!filePtr)
        throw omnetpp::cRuntimeError("Cannot create file '%s'", filePath.c_str());

    // write simulation parameters at the beginning of the file
    {
        // get the current config name
        std::string configName = omnetpp::getEnvir()->getConfigEx()->getVariable("configname");

        std::string iniFile = omnetpp::getEnvir()->getConfigEx()->getVariable("inifile");

        // PID of the simulation process
        std::string processid = omnetpp::getEnvir()->getConfigEx()->getVariable("processid");

        // globally unique identifier for the run, produced by
        // concatenating the configuration name, run number, date/time, etc.
        std::string runID = omnetpp::getEnvir()->getConfigEx()->getVariable("runid");

        // get number of total runs in this config
        int totalRun = omnetpp::getEnvir()->getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

        // get configuration name
        std::vector<std::string> iterVar = omnetpp::getEnvir()->getConfigEx()->getConfigChain(configName.c_str());

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "iniFile         %s\n", iniFile.c_str());
        fprintf (filePtr, "processID       %s\n", processid.c_str());
        fprintf (filePtr, "runID           %s\n", runID.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n", iterVar[0].c_str());
        fprintf (filePtr, "sim timeStep    %u ms\n", TraCI->simulationGetDelta());
        fprintf (filePtr, "startDateTime   %s\n", TraCI->simulationGetStartTime_str().c_str());
        fprintf (filePtr, "endDateTime     %s\n", TraCI->simulationGetEndTime_str().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", TraCI->simulationGetDuration_str().c_str());
    }

    // write header
    fprintf (filePtr, "%-15s","simTime");
    fprintf (filePtr, "%-12s","entryLane");
    fprintf (filePtr, "%-10s","TLid");
    fprintf (filePtr, "%-20s","firstDetectedTime");
    fprintf (filePtr, "%-20s","lastDetectedTime");
    fprintf (filePtr, "%-15s","passageTime");
    fprintf (filePtr, "%-20s","totalVehicleCount");
    fprintf (filePtr, "%-1s \n\n","vehicles");

    double oldTime = -2;

    // write body
    for(auto &y : vehApproachPerLane)
    {
        // iterate over incoming lanes
        for(auto &v : y.laneInfo)
        {
            auto vehs = v.second.allVehicles;

            if(vehs.empty())
                continue;

            if(oldTime != y.time)
            {
                fprintf(filePtr, "\n");
                oldTime = y.time;
            }

            fprintf (filePtr, "%-15.4f", y.time);
            fprintf (filePtr, "%-12s", v.first.c_str());
            fprintf (filePtr, "%-10s", v.second.TLid.c_str());
            fprintf (filePtr, "%-20.4f", v.second.firstDetectedTime);
            fprintf (filePtr, "%-20.4f", v.second.lastDetectedTime);
            fprintf (filePtr, "%-15.2f", v.second.passageTime);
            fprintf (filePtr, "%-20d", v.second.totalVehCount);

            for(auto &entry : vehs)
                fprintf (filePtr, "%-1s, ", entry.first.c_str());

            fprintf (filePtr, "\n");
        }
    }

    fclose(filePtr);
}

}

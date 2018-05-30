/****************************************************************************/
/// @file    IntersectionQueue.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author
/// @date    April 2015
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

#include "trafficLight/03_IntersectionDemand.h"

namespace VENTOS {

Define_Module(VENTOS::IntersectionQueue);


IntersectionQueue::~IntersectionQueue()
{

}


void IntersectionQueue::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        record_intersectionQueue_stat = par("record_intersectionQueue_stat").boolValue();

        speedThreshold_veh = par("speedThreshold_veh").doubleValue();
        speedThreshold_bike = par("speedThreshold_bike").doubleValue();

        if(speedThreshold_veh < 0)
            throw omnetpp::cRuntimeError("speedThreshold_veh is not set correctly!");

        if(speedThreshold_bike < 0)
            throw omnetpp::cRuntimeError("speedThreshold_bike is not set correctly!");

        queueSizeLimit = par("queueSizeLimit").intValue();

        if(queueSizeLimit <= 0 && queueSizeLimit != -1)
            throw omnetpp::cRuntimeError("queueSizeLimit value is set incorrectly!");
    }
}


void IntersectionQueue::finish()
{
    super::finish();

    saveTLQueueingData();
}


void IntersectionQueue::handleMessage(omnetpp::cMessage *msg)
{
    super::handleMessage(msg);
}


void IntersectionQueue::initialize_withTraCI()
{
    super::initialize_withTraCI();

    if(record_intersectionQueue_stat)
    {
        TLList = TraCI->TLGetIDList();

        if(TLList.empty())
            LOG_INFO << ">>> WARNING: no traffic light found in the network. \n" << std::flush;

        initVariables();
    }
}


void IntersectionQueue::executeEachTimeStep()
{
    super::executeEachTimeStep();

    if(record_intersectionQueue_stat)
    {
        updateQueuePerLane();

        updateQueuePerTL();
    }
}


void IntersectionQueue::initVariables()
{
    // for each traffic light
    for (auto &TLid : TLList)
    {
        // get all incoming lanes
        auto lan = TraCI->TLGetControlledLanes(TLid);

        // remove duplicate entries
        sort( lan.begin(), lan.end() );
        lan.erase( unique( lan.begin(), lan.end() ), lan.end() );

        incomingLanes_perTL[TLid] = lan;

        std::vector<std::string> sideWalkList;

        // for each incoming lane
        for(auto &lane : lan)
        {
            incomingLanes[lane] = TLid;

            // initialize queue value in laneQueueSize to zero
            queueInfoLane_t entry;
            entry.TLid = TLid;
            entry.queueSize = 0;
            entry.vehs = std::vector<vehInfo_t> ();

            queueInfo_perLane[lane] = entry;

            // store all bike lanes and side walks
            auto allowedClasses = TraCI->laneGetAllowedClasses(lane);
            if(allowedClasses.size() == 1 && allowedClasses.front() == "pedestrian")
                sideWalkList.push_back(lane);
        }

        sideWalks_perTL[TLid] = sideWalkList;
    }

    // iterate over all incoming lanes and remove sideWalks
    for (auto it = incomingLanes.begin(); it != incomingLanes.end() /* not hoisted */; /* no increment */)
    {
        std::string lane = (*it).first;
        std::string TLid = (*it).second;

        if( std::find(sideWalks_perTL[TLid].begin(), sideWalks_perTL[TLid].end(), lane) != sideWalks_perTL[TLid].end() )
            it = incomingLanes.erase(it);
        else
            ++it;
    }
}


// update queueInfo_perLane with the latest queue information
void IntersectionQueue::updateQueuePerLane()
{
    // for each 'lane i' that is controlled by traffic light j
    for(auto &y : incomingLanes)
    {
        std::string lane = y.first;
        std::string TLid = y.second;

        // get all vehicles on this incoming lane
        // note: a vehicle that crosses the intersection is not considered part of the incoming lane
        auto vehsOnLane = TraCI->laneGetLastStepVehicleIDs(lane);

        // get the vehicles that are waiting on this lane
        auto location = queueInfo_perLane.find(lane);
        auto &queuedVehs = location->second.vehs;

        // remove the vehicles in queuedVehs that are not in vehsOnLane anymore
        for (auto it = queuedVehs.begin(); it != queuedVehs.end() /* not hoisted */; /* no increment */)
        {
            auto search = std::find(vehsOnLane.begin(), vehsOnLane.end(), (*it).id);
            if (search == vehsOnLane.end())
                it = queuedVehs.erase(it);
            else
                ++it;
        }

        // iterate over vehicles starting from the end of lane (closer to the intersection)
        int vehCount = 0;
        for(auto rit = vehsOnLane.rbegin(); rit != vehsOnLane.rend(); rit++)
        {
            // if the vehicle is waiting
            auto search = std::find_if(queuedVehs.begin(), queuedVehs.end(), [&](const vehInfo_t &a){return a.id == *rit;});
            if(search != queuedVehs.end())
            {
                vehCount++;
                continue;
            }

            // treating the leading vehicle differently
            if(vehCount == 0)
            {
                std::vector<TL_info_t> nextTL = TraCI->vehicleGetNextTLS(*rit);

                // SUMO returns empty 'nextTL' for the leading vehicle.
                // this happens when the leading vehicle is changing lane on a wrong incoming lane
                if(nextTL.empty())
                    break;

                // queue start should be [0,10] meters from the intersection
                if(nextTL[0].TLS_distance > 10)
                    break;

                std::string vehType = TraCI->vehicleGetTypeID(*rit);

                double stoppingDelayThreshold = 0;
                if(vehType == "bicycle")
                    stoppingDelayThreshold = speedThreshold_bike;
                else
                    stoppingDelayThreshold = speedThreshold_veh;

                double speed = TraCI->vehicleGetSpeed(*rit);

                if(speed <= stoppingDelayThreshold)
                {
                    if(queueSizeLimit == -1 || (queueSizeLimit != -1 && queuedVehs.size() < (unsigned int)queueSizeLimit))
                    {
                        vehInfo_t entry = {*rit, vehType};
                        queuedVehs.push_back(entry);
                    }
                }

                vehCount++;
                continue;
            }

            // get the leading vehicle
            auto leader = TraCI->vehicleGetLeader(*rit, 10000);

            if(leader.distance2Leader > 10)
                break;

            std::string vehType = TraCI->vehicleGetTypeID(*rit);

            double stoppingDelayThreshold = 0;
            if(vehType == "bicycle")
                stoppingDelayThreshold = speedThreshold_bike;
            else
                stoppingDelayThreshold = speedThreshold_veh;

            double speed = TraCI->vehicleGetSpeed(*rit);

            if(speed <= stoppingDelayThreshold)
            {
                if(queueSizeLimit == -1 || (queueSizeLimit != -1 && queuedVehs.size() < (unsigned int)queueSizeLimit))
                {
                    vehInfo_t entry = {*rit, vehType};
                    queuedVehs.push_back(entry);
                }
            }

            vehCount++;
        }

        // update queue size in laneQueueSize
        location->second.queueSize = queuedVehs.size();
    }
}


// now that we have updated queueInfo_perLane, we can update queueInfo_perTL
void IntersectionQueue::updateQueuePerTL()
{
    for (auto &TLid : TLList)
    {
        auto it = incomingLanes_perTL.find(TLid);
        if(it == incomingLanes_perTL.end())
            throw omnetpp::cRuntimeError("cannot find %s in incomingLanes_perTL", TLid.c_str());

        int totalQueueSize = 0;
        int maxQueueSize = -1;
        int totalLanes = it->second.size();

        for(auto &lane : incomingLanes_perTL[TLid])
        {
            auto it2 = queueInfo_perLane.find(lane);
            if(it2 == queueInfo_perLane.end())
                throw omnetpp::cRuntimeError("cannot find %s in queueInfo_perLane", lane.c_str());
            int num = it2->second.queueSize;

            totalQueueSize += num;
            maxQueueSize = std::max(maxQueueSize, num);
        }

        auto it3 = queueInfo_perTL.find(TLid);
        if(it3 == queueInfo_perTL.end())
        {
            queueInfoTL_t entry = {};

            entry.time = omnetpp::simTime().dbl();
            entry.totalQueueSize = totalQueueSize;
            entry.maxQueueSize = maxQueueSize;
            entry.totalLanes = totalLanes;

            std::vector<queueInfoTL_t> infos = {entry};

            queueInfo_perTL[TLid] = infos;
        }
        else
        {
            // update queueInfo_perTL upon queue changes only
            auto &lastElement = it3->second.back();
            if(lastElement.totalQueueSize != totalQueueSize || lastElement.maxQueueSize != maxQueueSize)
            {
                queueInfoTL_t entry = {};

                entry.time = omnetpp::simTime().dbl();
                entry.totalQueueSize = totalQueueSize;
                entry.maxQueueSize = maxQueueSize;
                entry.totalLanes = totalLanes;

                it3->second.push_back(entry);
            }
        }
    }
}



queueInfoLane_t IntersectionQueue::laneGetQueue(std::string laneID)
{
    auto it = queueInfo_perLane.find(laneID);
    if(it == queueInfo_perLane.end())
        throw omnetpp::cRuntimeError("cannot find incoming lane '%s' in queueInfo_perLane", laneID.c_str());

    return it->second;
}


queueInfoTL_t IntersectionQueue::TLGetQueue(std::string TLid)
{
    auto it = queueInfo_perTL.find(TLid);
    if(it == queueInfo_perTL.end())
        throw omnetpp::cRuntimeError("cannot find TL '%s' in queueInfo_perTL", TLid.c_str());

    // the last entry in the vector is the latest queue info in this TL
    return it->second.back();
}


void IntersectionQueue::saveTLQueueingData()
{
    if(queueInfo_perTL.empty())
        return;

    for(auto &entry : queueInfo_perTL)
    {
        // ignore the intersection with zero queue size during simulation
        if(entry.second.size() == 1 && entry.second[0].maxQueueSize == 0)
            continue;

        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

        std::ostringstream fileName;
        fileName << boost::format("%03d_TLqueuingData_%s.txt") % currentRun % entry.first;

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
        fprintf (filePtr, "%-10s", "index");
        fprintf (filePtr, "%-10s", "timeStep");
        fprintf (filePtr, "%-10s", "TLid");
        fprintf (filePtr, "%-15s", "totalQueue");
        fprintf (filePtr, "%-15s", "maxQueue");
        fprintf (filePtr, "%-10s\n\n", "laneCount");

        double oldTime = -1;
        int index = 0;

        for(auto &y : entry.second)
        {
            if(oldTime != y.time)
            {
                fprintf(filePtr, "\n");
                oldTime = y.time;
                index++;
            }

            fprintf (filePtr, "%-10d", index);
            fprintf (filePtr, "%-10.2f", y.time);
            fprintf (filePtr, "%-10s", entry.first.c_str());
            fprintf (filePtr, "%-15d", y.totalQueueSize);
            fprintf (filePtr, "%-15d", y.maxQueueSize);
            fprintf (filePtr, "%-10d\n", y.totalLanes);
        }

        fclose(filePtr);
    }
}

}

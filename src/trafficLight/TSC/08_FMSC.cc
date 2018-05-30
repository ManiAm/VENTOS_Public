/****************************************************************************/
/// @file    FMSC.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @date    Jul 2015
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

#include <queue>

#include <trafficLight/TSC/08_FMSC.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLight_FMSC);


TrafficLight_FMSC::~TrafficLight_FMSC()
{

}


void TrafficLight_FMSC::initialize(int stage)
{
    if(par("TLControlMode").intValue() == TL_FMSC)
    {
        par("record_intersectionQueue_stat") = true;
        par("record_intersectionDelay_stat") = true;
    }

    super::initialize(stage);

    if(TLControlMode != TL_FMSC)
        return;

    if(stage == 0)
    {
        nextGreenIsNewCycle = false;
        intervalChangeEVT = new omnetpp::cMessage("intervalChangeEVT", 1);
    }
}


void TrafficLight_FMSC::finish()
{
    super::finish();
}


void TrafficLight_FMSC::handleMessage(omnetpp::cMessage *msg)
{
    if (TLControlMode == TL_FMSC && msg == intervalChangeEVT)
    {
        if(greenInterval.empty())
            calculatePhases("C");

        chooseNextInterval("C");

        if(intervalDuration <= 0)
            throw omnetpp::cRuntimeError("intervalDuration is <= 0");

        // Schedule next light change event:
        scheduleAt(omnetpp::simTime().dbl() + intervalDuration, intervalChangeEVT);
    }
    else
        super::handleMessage(msg);
}


void TrafficLight_FMSC::initialize_withTraCI()
{
    // call parent
    super::initialize_withTraCI();

    if(TLControlMode != TL_FMSC)
        return;

    LOG_INFO << "\nMulti-class LQF-MWM-Cycle traffic signal control ... \n" << std::flush;

    auto TLList = TraCI->TLGetIDList();

    // for each traffic light
    for (auto &TLid : TLList)
    {
        // get all incoming lanes
        auto lan = TraCI->TLGetControlledLanes(TLid);

        // remove duplicate entries
        sort( lan.begin(), lan.end() );
        lan.erase( unique( lan.begin(), lan.end() ), lan.end() );

        incomingLanes_perTL[TLid] = lan;

        // get all links controlled by this TL
        auto result = TraCI->TLGetControlledLinks(TLid);

        // for each link in this TLid
        for(auto &it2 : result)
        {
            int linkNumber = it2.first;
            std::vector<std::string> link = it2.second;
            std::string incommingLane = link[0];

            link2Lane.insert( std::make_pair(std::make_pair(TLid,linkNumber), incommingLane) );
        }
    }

    // calculate phases at the beginning of the cycle
    calculatePhases("C");

    // set initial settings:
    currentInterval = greenInterval.front().greenString;
    intervalDuration = greenInterval.front().greenTime;

    scheduleAt(omnetpp::simTime().dbl() + intervalDuration, intervalChangeEVT);

    for (auto &TL : TLList)
    {
        TraCI->TLSetProgram(TL, "adaptive-time");
        TraCI->TLSetState(TL, currentInterval);

        firstGreen[TL] = currentInterval;

        // initialize TL status
        updateTLstate(TL, "init", currentInterval);
    }

    LOG_DEBUG << boost::format("\n    SimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
    % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
}


void TrafficLight_FMSC::executeEachTimeStep()
{
    // call parent
    super::executeEachTimeStep();

    if(TLControlMode != TL_FMSC)
        return;
}


void TrafficLight_FMSC::chooseNextInterval(std::string TLid)
{
    if (currentInterval == "yellow")
    {
        currentInterval = "red";

        // change all 'y' to 'r'
        std::string str = TraCI->TLGetState(TLid);
        std::string nextInterval = "";
        for(char& c : str) {
            if (c == 'y')
                nextInterval += 'r';
            else
                nextInterval += c;
        }

        // set the new state
        TraCI->TLSetState(TLid, nextInterval);
        intervalDuration = redTime;

        // update TL status for this phase
        updateTLstate(TLid, "red");
    }
    else if (currentInterval == "red")
    {
        if(nextGreenIsNewCycle)
        {
            updateTLstate(TLid, "phaseEnd", nextGreenInterval, true);  // a new cycle
            nextGreenIsNewCycle = false;
        }
        else
            updateTLstate(TLid, "phaseEnd", nextGreenInterval);

        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState(TLid, nextGreenInterval);
        intervalDuration = greenInterval.front().greenTime;
    }
    else
        chooseNextGreenInterval(TLid);

    LOG_DEBUG << boost::format("\n    SimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
    % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
}


void TrafficLight_FMSC::chooseNextGreenInterval(std::string TLid)
{
    // Remove current old phase:
    greenInterval.erase(greenInterval.begin());

    // Assign new green:
    if (greenInterval.empty())
    {
        calculatePhases(TLid);
        nextGreenIsNewCycle = true;
    }

    nextGreenInterval = greenInterval.front().greenString;

    // calculate 'next interval'
    std::string nextInterval = "";
    for(unsigned int linkNumber = 0; linkNumber < currentInterval.size(); ++linkNumber)
    {
        if( (currentInterval[linkNumber] == 'G' || currentInterval[linkNumber] == 'g') && nextGreenInterval[linkNumber] == 'r')
            nextInterval += 'y';
        else
            nextInterval += currentInterval[linkNumber];
    }

    currentInterval = "yellow";
    TraCI->TLSetState(TLid, nextInterval);

    intervalDuration =  yellowTime;

    // update TL status for this phase
    updateTLstate(TLid, "yellow");
}


void TrafficLight_FMSC::calculatePhases(std::string TLid)
{
    LOG_DEBUG << "\n------------------------------------------------------------------------------- \n";
    LOG_DEBUG << ">>> New cycle calculation \n" << std::flush;

    if(LOG_ACTIVE(DEBUG_LOG_VAL))
    {
        auto it = incomingLanes_perTL.find(TLid);
        if(it == incomingLanes_perTL.end())
            throw omnetpp::cRuntimeError("cannot find TLid '%s' in incomingLanes_perTL", TLid.c_str());

        LOG_DEBUG << "\n    Queue size per lane: ";
        for(auto &lane : it->second)
        {
            int qSize = laneGetQueue(lane).queueSize;
            if(qSize != 0)
                LOG_DEBUG << lane << " (" << qSize << ") | ";
        }

        LOG_DEBUG << "\n\n    Total delay of vehicles on each lane: \n";
        for(auto &lane : it->second)
        {
            auto vehs = laneGetQueue(lane).vehs;

            double totalDelay = 0;
            for(auto &veh : vehs)
            {
                delayEntry_t *vehDelay = vehicleGetDelay(veh.id,TLid);
                if(vehDelay)
                    totalDelay += vehDelay->totalDelay;
            }

            if(totalDelay != 0)
                LOG_DEBUG << boost::format("        lane %1%: %2% \n") % lane % totalDelay;
        }

        LOG_FLUSH;
    }

    priorityQ sortedMovements;

    for(std::string phase : phases)
    {
        double totalWeight = 0;  // total weight of all waiting entities in all movements in this phase
        double totalDelay = 0;  // total delay of all waiting entities in all movements in this phase
        int oneCount = 0;
        int maxVehCount = 0;

        for(unsigned int linkNumber = 0; linkNumber < phase.size(); ++linkNumber)
        {
            int vehCount = 0;
            if(phase[linkNumber] == 'G')
            {
                // ignore this link if right turn
                if(!isRightTurn(linkNumber))
                {
                    // get the corresponding lane for this link
                    auto itt = link2Lane.find(std::make_pair(TLid,linkNumber));
                    if(itt == link2Lane.end())
                        throw omnetpp::cRuntimeError("linkNumber %s is not found in TL %s", linkNumber, TLid.c_str());
                    std::string lane = itt->second;

                    auto queuedVehs = laneGetQueue(lane).vehs;

                    for(auto &veh : queuedVehs)
                    {
                        // total weight of entities on this lane
                        auto loc = classWeight.find(veh.type);
                        if(loc == classWeight.end())
                            throw omnetpp::cRuntimeError("entity %s with type %s does not have a weight in classWeight map!", veh.id.c_str(), veh.type.c_str());

                        totalWeight += loc->second;

                        // total delay in this lane
                        delayEntry_t *vehDelay = vehicleGetDelay(veh.id,TLid);
                        if(vehDelay)
                            totalDelay += vehDelay->totalDelay;

                        vehCount++;
                    }
                }

                // number of movements in this phase (including right turns)
                oneCount++;
            }

            maxVehCount = std::max(maxVehCount, vehCount);
        }

        // add this batch of movements to priority_queue
        sortedEntry_t entry = {totalWeight, totalDelay, oneCount, maxVehCount, phase};
        sortedMovements.push(entry);
    }

    // copy sortedMovements to a vector for iteration:
    std::vector<sortedEntry_t> batchMovementVector;
    while(!sortedMovements.empty())
    {
        batchMovementVector.push_back(sortedMovements.top());
        sortedMovements.pop();
    }

    // Select only the necessary phases for the new cycle:
    while(!batchMovementVector.empty())
    {
        // Always select the first movement because it will be the best(?):
        sortedEntry_t entry = batchMovementVector.front();
        std::string nextInterval = entry.phase;

        greenIntervalEntry_t entry2 = {entry.maxVehCount, entry.totalWeight, entry.oneCount, 0.0, entry.phase};
        greenInterval.push_back(entry2);

        // Now delete these movements because they should never occur again:
        auto new_end = std::remove_if(batchMovementVector.begin(), batchMovementVector.end(), [nextInterval](const sortedEntry_t v) {

            std::string phase = v.phase;

            for(unsigned int i = 0; i < phase.size(); i++)
            {
                if (phase[i] == 'G' && nextInterval[i] == 'G')
                    return true;
            }

            return false;
        });

        batchMovementVector.erase(new_end, batchMovementVector.end());
    }

    // calculate number of vehicles in the intersection
    int vehCountIntersection = 0;
    for (auto &i : greenInterval)
        vehCountIntersection += i.maxVehCount;

    // If intersection is empty, then run each green interval with minGreenTime:
    if (vehCountIntersection == 0)
    {
        for (auto &i : greenInterval)
            i.greenTime = minGreenTime;
    }
    else
    {
        for (auto &i : greenInterval)
            i.greenTime = (double)i.maxVehCount * (minGreenTime / 5.);
    }

    // If no green time (0s) is given to a phase, then this queue is empty and useless:
    int oldSize = greenInterval.size();
    auto rme = std::remove_if(greenInterval.begin(), greenInterval.end(),
            [](const greenIntervalEntry_t v) {
        if (v.greenTime == 0.0)
            return true;
        else
            return false;
    });

    greenInterval.erase(rme, greenInterval.end());

    int newSize = greenInterval.size();
    if(oldSize != newSize)
        LOG_DEBUG << "\n    " << oldSize - newSize << " phase(s) removed due to zero queue size! \n" << std::flush;

    // make sure the green splits are bounded
    for (auto &i : greenInterval)
        i.greenTime = std::min(std::max(i.greenTime, minGreenTime), maxGreenTime);

    if(LOG_ACTIVE(DEBUG_LOG_VAL))
    {
        LOG_DEBUG << "\n    Selected green intervals for this cycle: \n";
        for (auto &i : greenInterval)
            LOG_DEBUG << "        movement: " << i.greenString
            << ", maxVehCount= " << i.maxVehCount
            << ", totalWeight= " << i.totalWeight
            << ", oneCount= " << i.oneCount
            << ", green= " << i.greenTime << "s \n";

        LOG_FLUSH;
    }

    LOG_DEBUG << "------------------------------------------------------------------------------- \n" << std::flush;
}

}

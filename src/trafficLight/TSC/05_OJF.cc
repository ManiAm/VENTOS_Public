/****************************************************************************/
/// @file    OJF.cc
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

#include <algorithm>
#include <iomanip>
#include <queue>

#include "trafficLight/TSC/05_OJF.h"

namespace VENTOS {

Define_Module(VENTOS::TrafficLightOJF);


TrafficLightOJF::~TrafficLightOJF()
{

}


void TrafficLightOJF::initialize(int stage)
{
    if(par("TLControlMode").intValue() == TL_OJF)
        par("record_intersectionDelay_stat") = true;

    super::initialize(stage);

    if(TLControlMode != TL_OJF)
        return;

    if(stage == 0)
    {
        intervalChangeEVT = new omnetpp::cMessage("intervalChangeEVT", 1);
    }
}


void TrafficLightOJF::finish()
{
    super::finish();
}


void TrafficLightOJF::handleMessage(omnetpp::cMessage *msg)
{
    if (TLControlMode == TL_OJF && msg == intervalChangeEVT)
    {
        chooseNextInterval("C");

        if(intervalDuration <= 0)
            throw omnetpp::cRuntimeError("intervalDuration is <= 0");

        // Schedule next light change event:
        scheduleAt(omnetpp::simTime().dbl() + intervalDuration, intervalChangeEVT);
    }
    else
        super::handleMessage(msg);
}


void TrafficLightOJF::initialize_withTraCI()
{
    super::initialize_withTraCI();

    if(TLControlMode != TL_OJF)
        return;

    LOG_INFO << "\nLow delay traffic signal control ...  \n" << std::flush;

    auto TLList = TraCI->TLGetIDList();
    for (auto &TL : TLList)
    {
        // get all incoming lanes
        auto lan = TraCI->TLGetControlledLanes(TL);

        // remove duplicate entries
        sort( lan.begin(), lan.end() );
        lan.erase( unique( lan.begin(), lan.end() ), lan.end() );

        incomingLanes_perTL[TL] = lan;

        // get all links controlled by this TL
        auto result = TraCI->TLGetControlledLinks(TL);

        // for each link in this TLid
        for(auto &it : result)
        {
            int linkNumber = it.first;
            std::vector<std::string> link = it.second;
            std::string incommingLane = link[0];

            link2Lane.insert( std::make_pair(std::make_pair(TL,linkNumber), incommingLane) );
        }
    }

    // set initial values
    currentInterval = phase1_5;
    intervalDuration = minGreenTime;

    scheduleAt(omnetpp::simTime().dbl() + intervalDuration, intervalChangeEVT);

    // get all non-conflicting movements in allMovements vector
    allMovements = TrafficLightAllowedMoves::getMovements("C");

    for (auto &TL : TLList)
    {
        TraCI->TLSetProgram(TL, "adaptive-time");
        TraCI->TLSetState(TL, currentInterval);

        firstGreen[TL] = currentInterval;

        // initialize TL status
        updateTLstate(TL, "init", currentInterval);
    }

    LOG_DEBUG << boost::format("\nSimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
    % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
}


void TrafficLightOJF::executeEachTimeStep()
{
    super::executeEachTimeStep();

    if(TLControlMode != TL_OJF)
        return;
}


void TrafficLightOJF::chooseNextInterval(std::string TLid)
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
        // update TL status for this phase
        if(nextGreenInterval == firstGreen[TLid])
            updateTLstate(TLid, "phaseEnd", nextGreenInterval, true);  // todo: notion of cycle?
        else
            updateTLstate(TLid, "phaseEnd", nextGreenInterval);

        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState(TLid, nextGreenInterval);
        intervalDuration = nextGreenTime;
    }
    else
        chooseNextGreenInterval(TLid);

    LOG_DEBUG << boost::format("\nSimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
    % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
}


void TrafficLightOJF::chooseNextGreenInterval(std::string TLid)
{
    LOG_DEBUG << "\n------------------------------------------------------------------------------- \n";
    LOG_DEBUG << ">>> New phase calculation \n" << std::flush;

    // for debugging
    if(LOG_ACTIVE(DEBUG_LOG_VAL))
    {
        LOG_DEBUG << "\n    Total delay of vehicles on each lane: \n";

        auto it = incomingLanes_perTL.find(TLid);
        if(it == incomingLanes_perTL.end())
            throw omnetpp::cRuntimeError("cannot find TL '%s'", TLid.c_str());

        // iterate over incoming lanes
        for(auto &lane : it->second)
        {
            auto vehs = TraCI->laneGetLastStepVehicleIDs(lane);

            double totalDelay = 0;
            for(auto &veh : vehs)
            {
                delayEntry_t *vehDelay = vehicleGetDelay(veh,TLid);
                if(vehDelay)
                    totalDelay += vehDelay->totalDelay;
            }

            if(totalDelay != 0)
                LOG_DEBUG << boost::format("        lane %1%: %2% \n") % lane % totalDelay;
        }
        LOG_FLUSH;
    }

    priorityQ sortedMovements;

    // Calculate totalDelay and vehCount for each movement batch
    for(unsigned int i = 0; i < allMovements.size(); ++i)  // row
    {
        double totalDelayRow = 0;
        int oneCount = 0;
        int maxVehCount = 0;

        for(unsigned int linkNumber = 0; linkNumber < allMovements[i].size(); ++linkNumber)  // column (link number)
        {
            int vehCount = 0;
            if(allMovements[i][linkNumber] == 1)
            {
                // ignore this link if right turn
                if(!isRightTurn(linkNumber))
                {
                    // get the corresponding lane for this link
                    auto itt = link2Lane.find(std::make_pair(TLid,linkNumber));
                    if(itt == link2Lane.end())
                        throw omnetpp::cRuntimeError("linkNumber %s is not found in TL %s", linkNumber, TLid.c_str());
                    std::string lane = itt->second;

                    // get all vehicles on this lane
                    auto vehs = TraCI->laneGetLastStepVehicleIDs(lane);

                    for(auto &veh :vehs)
                    {
                        // total delay in this lane
                        delayEntry_t *vehDelay = vehicleGetDelay(veh,TLid);
                        if(vehDelay)
                        {
                            totalDelayRow += vehDelay->totalDelay;

                            if(vehDelay->totalDelay > 0)
                                vehCount++;
                        }
                    }
                }

                oneCount++;
            }

            maxVehCount = std::max(maxVehCount, vehCount);
        }

        // add this movement batch to priority_queue
        sortedEntry_t entry = {oneCount, maxVehCount, totalDelayRow, allMovements[i]};
        sortedMovements.push(entry);
    }

    // get the movement batch with the highest delay
    sortedEntry_t entry = sortedMovements.top();

    // allocate enough green time to move all delayed vehicle
    int maxVehCount = entry.maxVehCount;
    LOG_DEBUG << "\n    Maximum of " << maxVehCount << " vehicle(s) are waiting. \n" << std::flush;

    double greenTime = (double)maxVehCount * (minGreenTime / 5.);
    nextGreenTime = std::min(std::max(greenTime, minGreenTime), maxGreenTime);  // bound green time

    std::vector<int> batchMovements = entry.batchMovements;

    // calculate the next green interval.
    // right-turns are all permissive and are given 'g'
    nextGreenInterval = "";
    for(unsigned int linkNumber = 0; linkNumber < batchMovements.size(); ++linkNumber)
    {
        if(batchMovements[linkNumber] == 0)
            nextGreenInterval += 'r';
        else if(batchMovements[linkNumber] == 1)
        {
            if(isRightTurn(linkNumber))
                nextGreenInterval += 'g';
            else
                nextGreenInterval += 'G';
        }
    }

    // calculate 'next interval'
    std::string nextInterval = "";
    bool needYellowInterval = false;  // if we have at least one yellow interval
    for(unsigned int linkNumber = 0; linkNumber < batchMovements.size(); ++linkNumber)
    {
        if( (currentInterval[linkNumber] == 'G' || currentInterval[linkNumber] == 'g') && nextGreenInterval[linkNumber] == 'r')
        {
            nextInterval += 'y';
            needYellowInterval = true;
        }
        else
            nextInterval += currentInterval[linkNumber];
    }

    if(needYellowInterval)
    {
        currentInterval = "yellow";
        TraCI->TLSetState(TLid, nextInterval);

        intervalDuration =  yellowTime;

        LOG_DEBUG << boost::format("\n    The next green interval is %s \n") % nextGreenInterval;

        // update TL status for this phase
        updateTLstate(TLid, "yellow");
    }
    else
    {
        intervalDuration = nextGreenTime;
        LOG_DEBUG << "\n    Continue the last green interval. \n" << std::flush;
    }

    LOG_DEBUG << "------------------------------------------------------------------------------- \n" << std::flush;
}

}

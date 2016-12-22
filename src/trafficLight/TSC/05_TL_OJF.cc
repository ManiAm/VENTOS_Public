/****************************************************************************/
/// @file    TL_OJF.cc
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

#include "05_TL_OJF.h"

#include <algorithm>
#include <iomanip>
#include <queue>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightOJF);

class sortedEntryD
{
public:
    int oneCount;
    int maxVehCount;
    double totalDelay;
    std::vector<int> batchMovements;

    sortedEntryD(int i1, int i2, double d1, std::vector<int> bm)
    {
        this->oneCount = i1;
        this->maxVehCount = i2;
        this->totalDelay = d1;
        batchMovements.swap(bm);
    }
};


class sortCompareD
{
public:
    bool operator()(sortedEntryD p1, sortedEntryD p2)
    {
        if( p1.totalDelay < p2.totalDelay )
            return true;
        else if( p1.totalDelay == p2.totalDelay && p1.oneCount < p2.oneCount)
            return true;
        else
            return false;
    }
};


TrafficLightOJF::~TrafficLightOJF()
{

}


void TrafficLightOJF::initialize(int stage)
{
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
        chooseNextInterval();

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

    // set initial values
    currentInterval = phase1_5;
    intervalDuration = minGreenTime;

    scheduleAt(omnetpp::simTime().dbl() + intervalDuration, intervalChangeEVT);

    // get all non-conflicting movements in allMovements vector
    TrafficLightAllowedMoves::getMovements("C");

    // make sure allMovements vector is not empty
    ASSERT(!allMovements.empty());

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


void TrafficLightOJF::chooseNextInterval()
{
    if (currentInterval == "yellow")
    {
        currentInterval = "red";

        // change all 'y' to 'r'
        std::string str = TraCI->TLGetState("C");
        std::string nextInterval = "";
        for(char& c : str) {
            if (c == 'y')
                nextInterval += 'r';
            else
                nextInterval += c;
        }

        // set the new state
        TraCI->TLSetState("C", nextInterval);
        intervalDuration = redTime;

        // update TL status for this phase
        updateTLstate("C", "red");
    }
    else if (currentInterval == "red")
    {
        // update TL status for this phase
        if(nextGreenInterval == firstGreen["C"])
            updateTLstate("C", "phaseEnd", nextGreenInterval, true);  // todo: notion of cycle?
        else
            updateTLstate("C", "phaseEnd", nextGreenInterval);

        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState("C", nextGreenInterval);
        intervalDuration = nextGreenTime;
    }
    else
        chooseNextGreenInterval();

    LOG_DEBUG << boost::format("\n    SimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
    % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
}


void TrafficLightOJF::chooseNextGreenInterval()
{
    LOG_DEBUG << "\n>>> New phase calculation ... \n" << std::flush;

    // for debugging
    if(LOG_ACTIVE(DEBUG_LOG_VAL))
    {
        LOG_DEBUG << "\n    Accumulated delay of vehicles on each lane: \n";
        for(auto &y : laneDelay)
        {
            std::map<std::string,double> vehs = y.second;

            if(vehs.empty())
                continue;

            LOG_DEBUG << "        " << y.first << ": ";

            double totalDelay = 0;
            for(auto &z : vehs)
            {
                LOG_DEBUG << boost::format("%1% (%2%), ") % z.first % z.second;
                totalDelay = totalDelay + z.second;
            }

            LOG_DEBUG << " --> total delay = " << totalDelay << "\n";
        }
        LOG_FLUSH;
    }

    // batch of all non-conflicting movements, sorted by total vehicle delay per batch
    std::priority_queue< sortedEntryD /*type of each element*/, std::vector<sortedEntryD> /*container*/, sortCompareD > sortedMovements;
    // clear the priority queue
    sortedMovements = std::priority_queue < sortedEntryD, std::vector<sortedEntryD>, sortCompareD >();

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
                bool notRightTurn = std::find(std::begin(rightTurns), std::end(rightTurns), linkNumber) == std::end(rightTurns);
                // ignore this link if right turn
                if(notRightTurn)
                {
                    std::map<std::string /*vehID*/, double /*accum delay of vehID*/> vehs = linkDelay[std::make_pair("C",linkNumber)];

                    for(auto &it :vehs)
                    {
                        totalDelayRow = totalDelayRow + it.second;
                        vehCount++;
                    }
                }

                oneCount++;
            }

            maxVehCount = std::max(maxVehCount, vehCount);
        }

        // add this movement batch to priority_queue
        sortedEntryD *entry = new sortedEntryD(oneCount, maxVehCount, totalDelayRow, allMovements[i]);
        sortedMovements.push(*entry);
    }

    // get the movement batch with the highest delay
    sortedEntryD entry = sortedMovements.top();

    // allocate enough green time to move all delayed vehicle
    int maxVehCount = entry.maxVehCount;
    LOG_DEBUG << "\n    Maximum of " << maxVehCount << " vehicle(s) are waiting. \n" << std::flush;

    double greenTime = (double)maxVehCount * (minGreenTime / 5.);
    nextGreenTime = std::min(std::max(greenTime, minGreenTime), maxGreenTime);  // bound green time
    LOG_DEBUG << "\n    Next green time is " << nextGreenTime << "\n" << std::flush;

    std::vector<int> batchMovements = entry.batchMovements;

    // calculate the next green interval.
    // right-turns are all permissive and are given 'g'
    nextGreenInterval = "";
    for(unsigned int linkNumber = 0; linkNumber < batchMovements.size(); ++linkNumber)
    {
        // if a right turn
        bool rightTurn = std::find(std::begin(rightTurns), std::end(rightTurns), linkNumber) != std::end(rightTurns);

        if(batchMovements[linkNumber] == 0)
            nextGreenInterval += 'r';
        else if(batchMovements[linkNumber] == 1 && rightTurn)
            nextGreenInterval += 'g';
        else if(batchMovements[linkNumber] == 1 && !rightTurn)
            nextGreenInterval += 'G';
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
        TraCI->TLSetState("C", nextInterval);

        intervalDuration =  yellowTime;

        // update TL status for this phase
        updateTLstate("C", "yellow");
    }
    else
    {
        intervalDuration = nextGreenTime;
        LOG_DEBUG << "\n    Continue the last green interval. \n" << std::flush;
    }
}

}

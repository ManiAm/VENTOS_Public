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

#include <09_TL_OJF.h>
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
    TrafficLightLQF_NoStarv::initialize(stage);

    if(TLControlMode != TL_OJF)
        return;

    if(stage == 0)
    {
        intervalChangeEVT = new cMessage("intervalChangeEVT", 1);
    }
}


void TrafficLightOJF::finish()
{
    TrafficLightLQF_NoStarv::finish();
}


void TrafficLightOJF::handleMessage(cMessage *msg)
{
    TrafficLightLQF_NoStarv::handleMessage(msg);

    if(TLControlMode != TL_OJF)
        return;

    if (msg == intervalChangeEVT)
    {
        chooseNextInterval();

        if(intervalDuration <= 0)
            error("intervalDuration is <= 0");

        // Schedule next light change event:
        scheduleAt(simTime().dbl() + intervalDuration, intervalChangeEVT);
    }
}


void TrafficLightOJF::executeFirstTimeStep()
{
    TrafficLightLQF_NoStarv::executeFirstTimeStep();

    if(TLControlMode != TL_OJF)
        return;

    std::cout << endl << "Low delay traffic signal control ... " << endl << endl;

    // set initial values
    currentInterval = phase1_5;
    intervalDuration = minGreenTime;

    scheduleAt(simTime().dbl() + intervalDuration, intervalChangeEVT);

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

    if(ev.isGUI() && debugLevel > 0)
    {
        char buff[300];
        sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalDuration);
        std::cout << buff << endl << endl;
        std::cout.flush();
    }
}


void TrafficLightOJF::executeEachTimeStep()
{
    TrafficLightLQF_NoStarv::executeEachTimeStep();

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
            updateTLstate("C", "phaseEnd", nextGreenInterval, true);   // todo: the notion of cycle?
        else
            updateTLstate("C", "phaseEnd", nextGreenInterval);

        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState("C", nextGreenInterval);
        intervalDuration = nextGreenTime;
    }
    else
        chooseNextGreenInterval();

    if(ev.isGUI() && debugLevel > 0)
    {
        char buff[300];
        sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalDuration);
        std::cout << buff << endl << endl;
        std::cout.flush();
    }
}


void TrafficLightOJF::chooseNextGreenInterval()
{
    // for debugging
    if(ev.isGUI() && debugLevel > 1)
    {
        std::cout << "Accumulated delay of vehicles on each lane: " << endl;
        for(auto &y : laneDelay)
        {
            std::map<std::string,double> vehs = y.second;

            if(vehs.empty())
                continue;

            std::cout << y.first << ": ";

            double totalDelay = 0;
            for(auto &z : vehs)
            {
                std::cout << z.first << ", " << std::setw(5) << z.second << " | ";
                totalDelay = totalDelay + z.second;
            }

            std::cout << " --> total delay = " << totalDelay << endl;
        }
        std::cout << endl;
        std::cout.flush();
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

    // allocate enough green time to move all delayed vehicle
    int maxVehCount = entry.maxVehCount;
    if(ev.isGUI() && debugLevel > 1)
    {
        std::cout << "Maximum of " << maxVehCount << " vehicle(s) are waiting. ";
        std::cout.flush();
    }
    double greenTime = (double)maxVehCount * (minGreenTime / 5.);
    nextGreenTime = std::min(std::max(greenTime, minGreenTime), maxGreenTime);  // bound green time
    if(ev.isGUI() && debugLevel > 1)
    {
        std::cout << "Next green time is " << nextGreenTime << endl << endl;
        std::cout.flush();
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
        if(ev.isGUI() && debugLevel > 0)
        {
            std::cout << ">>> Continue the last green interval." << endl << endl;
            std::cout.flush();
        }
    }
}

}

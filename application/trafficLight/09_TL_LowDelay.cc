/****************************************************************************/
/// @file    TL_LowDelay.cc
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

#include <09_TL_LowDelay.h>
#include <iomanip>
#include <algorithm>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightLowDelay);

class batchMovementDelayEntry
{
public:
    int oneCount;
    int maxVehCount;
    double totalDelay;
    std::vector<int> batchMovements;

    batchMovementDelayEntry(int i1, int i2, double d1, std::vector<int> bm)
    {
        this->oneCount = i1;
        this->maxVehCount = i2;
        this->totalDelay = d1;
        batchMovements.swap(bm);
    }
};


class movementCompareDelay
{
public:
    bool operator()(batchMovementDelayEntry p1, batchMovementDelayEntry p2)
    {
        if( p1.totalDelay < p2.totalDelay )
            return true;
        else if( p1.totalDelay == p2.totalDelay && p1.oneCount < p2.oneCount)
            return true;
        else
            return false;
    }
};


TrafficLightLowDelay::~TrafficLightLowDelay()
{

}


void TrafficLightLowDelay::initialize(int stage)
{
    TrafficLightAdaptiveQueue::initialize(stage);

    if(TLControlMode != TL_LowDelay)
        return;

    if(stage == 0)
    {
        ChangeEvt = new cMessage("ChangeEvt", 1);
    }
}


void TrafficLightLowDelay::finish()
{
    TrafficLightAdaptiveQueue::finish();
}


void TrafficLightLowDelay::handleMessage(cMessage *msg)
{
    TrafficLightAdaptiveQueue::handleMessage(msg);

    if(TLControlMode != TL_LowDelay)
        return;

    if (msg == ChangeEvt)
    {
        chooseNextInterval();

        if(intervalOffSet <= 0)
            error("intervalOffSet is <= 0");

        // Schedule next light change event:
        scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);
    }
}


void TrafficLightLowDelay::executeFirstTimeStep()
{
    TrafficLightAdaptiveQueue::executeFirstTimeStep();

    if(TLControlMode != TL_LowDelay)
        return;

    std::cout << endl << "Low delay traffic signal control ... " << endl << endl;

    // set initial values
    currentInterval = phase1_5;
    intervalOffSet = minGreenTime;
    intervalElapseTime = 0;

    scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);

    // get all non-conflicting movements in allMovements vector
    TrafficLightAllowedMoves::getMovements("C");

    for (std::list<std::string>::iterator TL = TLList.begin(); TL != TLList.end(); ++TL)
    {
        TraCI->TLSetProgram(*TL, "adaptive-time");
        TraCI->TLSetState(*TL, currentInterval);

        firstGreen[*TL] = currentInterval;

        // initialize TL status
        updateTLstate(*TL, "init", currentInterval);
    }

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl << endl;
}


void TrafficLightLowDelay::executeEachTimeStep(bool simulationDone)
{
    TrafficLightAdaptiveQueue::executeEachTimeStep(simulationDone);

    if(TLControlMode != TL_LowDelay)
        return;

    intervalElapseTime += updateInterval;
}


void TrafficLightLowDelay::chooseNextInterval()
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
        intervalElapseTime = 0.0;
        intervalOffSet = redTime;

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
        intervalElapseTime = 0.0;
        intervalOffSet = nextGreenTime;
    }
    else
        chooseNextGreenInterval();

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl << endl;
}


void TrafficLightLowDelay::chooseNextGreenInterval()
{
    // for debugging
    std::cout << "Accumulated delay of vehicles on each lane: " << endl;
    for(std::map<std::string, std::map<std::string, double>>::iterator y = laneDelay.begin(); y != laneDelay.end(); ++y)
    {
        std::map<std::string,double> vehs = (*y).second;

        if(vehs.empty())
            continue;

        std::cout << (*y).first << ": ";

        double totalDelay = 0;
        for(std::map<std::string, double>::iterator z = vehs.begin(); z != vehs.end(); ++z)
        {
            std::cout << (*z).first << ", " << std::setw(5) << (*z).second << " | ";
            totalDelay = totalDelay + (*z).second;
        }

        std::cout << " --> total delay = " << totalDelay << endl;
    }
    std::cout << endl;

    // batch of all non-conflicting movements, sorted by total vehicle delay per batch
    std::priority_queue< batchMovementDelayEntry /*type of each element*/, std::vector<batchMovementDelayEntry> /*container*/, movementCompareDelay > batchMovementDelay;

    // clear the priority queue
    batchMovementDelay = std::priority_queue < batchMovementDelayEntry, std::vector<batchMovementDelayEntry>, movementCompareDelay >();

    // get which row has the highest delay
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

                    for(std::map<std::string, double>::iterator it = vehs.begin(); it != vehs.end(); ++it)
                    {
                        totalDelayRow = totalDelayRow + (*it).second;
                        vehCount++;
                    }
                }

                oneCount++;
            }

            maxVehCount = std::max(maxVehCount, vehCount);
        }

        // add this batch of movements to priority_queue
        batchMovementDelayEntry *entry = new batchMovementDelayEntry(oneCount, maxVehCount, totalDelayRow, allMovements[i]);
        batchMovementDelay.push(*entry);
    }

    // get the movement batch with the highest delay
    batchMovementDelayEntry entry = batchMovementDelay.top();
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
        else
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
    double greenTime = (double)maxVehCount * (minGreenTime / 5.);
    // bound green time
    nextGreenTime = std::min(std::max(greenTime, minGreenTime), maxGreenTime);
    std::cout << "Maximum of " << maxVehCount << " vehicles are waiting. ";
    std::cout << "Next green time is " << nextGreenTime << endl << endl;

    if(needYellowInterval)
    {
        currentInterval = "yellow";
        TraCI->TLSetState("C", nextInterval);

        intervalElapseTime = 0.0;
        intervalOffSet =  yellowTime;

        // update TL status for this phase
        updateTLstate("C", "yellow");
    }
    else
    {
        intervalOffSet = nextGreenTime;
        std::cout << ">>> Continue the last green interval." << endl << endl;
    }
}

}

/****************************************************************************/
/// @file    TL_MaxQueue.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
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

#include <09_TL_MaxQueue.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightAdaptiveQueue);

class batchMovementQueueEntry
{
public:
    int oneCount;
    int totalQueue;
    int maxVehCount;
    std::vector<int> batchMovements;

    batchMovementQueueEntry(int i1, int i2, int i3, std::vector<int> bm)
    {
        this->oneCount = i1;
        this->totalQueue = i2;
        this->maxVehCount = i3;
        batchMovements.swap(bm);
    }
};


class movementCompareQueue
{
public:
    bool operator()(batchMovementQueueEntry p1, batchMovementQueueEntry p2)
    {
        if(p1.totalQueue < p2.totalQueue)
            return true;
        else if(p1.totalQueue == p2.totalQueue && p1.oneCount < p2.oneCount)
            return true;
        else
            return false;
    }
};


// using a 'functor' rather than a 'function'
// Reason: to be able to pass an additional argument (bestMovement) to predicate
struct served
{
public:
    served(std::vector<int> best)
{
        bestMovement.swap(best);
}

    bool operator () (const batchMovementQueueEntry v)
    {
        for (unsigned int linkNumber = 0; linkNumber < v.batchMovements.size(); linkNumber++)
        {
            // Ignore all permissive right turns since these are always green:
            int rightTurns[8] = {0, 2, 5, 7, 10, 12, 15, 17};
            bool rightTurn = std::find(std::begin(rightTurns), std::end(rightTurns), linkNumber) != std::end(rightTurns);

            // Want to remove movements that have already been done:
            if (!rightTurn && v.batchMovements[linkNumber] == 1 && bestMovement[linkNumber] == 1)
                return true;
        }

        return false;
    }

private:
    std::vector<int> bestMovement;
};


bool noGreenTime(greenIntervalInfo v)
{
    if (v.greenTime == 0.0)
        return true;
    else
        return false;
}


TrafficLightAdaptiveQueue::~TrafficLightAdaptiveQueue()
{

}


void TrafficLightAdaptiveQueue::initialize(int stage)
{
    TrafficLightLowDelay::initialize(stage);

    if(TLControlMode != TL_Adaptive_Time_Queue)
        return;

    if(stage == 0)
    {
        maxQueueSize = par("maxQueueSize").longValue();

        if(maxQueueSize < -1)
            error("maxQueueSize value is set incorrectly!");

        nextGreenIsNewCycle = false;
        ChangeEvt = new cMessage("ChangeEvt", 1);

        // collect queue information
        measureIntersectionQueue = true;
    }
}


void TrafficLightAdaptiveQueue::finish()
{
    TrafficLightLowDelay::finish();
}


void TrafficLightAdaptiveQueue::handleMessage(cMessage *msg)
{
    TrafficLightLowDelay::handleMessage(msg);

    if(TLControlMode != TL_Adaptive_Time_Queue)
        return;

    if (msg == ChangeEvt)
    {
        if(greenInterval.empty())
            calculatePhases("C");

        chooseNextInterval();

        if(intervalOffSet <= 0)
            error("intervalOffSet is <= 0");

        // Schedule next light change event:
        scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);
    }
}


void TrafficLightAdaptiveQueue::executeFirstTimeStep()
{
    TrafficLightLowDelay::executeFirstTimeStep();

    if(TLControlMode != TL_Adaptive_Time_Queue)
        return;

    std::cout << endl << "Maximum queue traffic signal control ..." << endl << endl;

    // get all non-conflicting movements in allMovements vector
    TrafficLightAllowedMoves::getMovements("C");

    // calculate phases at the beginning of the cycle
    calculatePhases("C");

    // set initial settings:
    currentInterval = greenInterval.front().greenString;
    intervalOffSet = greenInterval.front().greenTime;
    intervalElapseTime = 0;

    scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);

    for (std::list<std::string>::iterator TL = TLList.begin(); TL != TLList.end(); ++TL)
    {
        TraCI->TLSetProgram(*TL, "adaptive-time");
        TraCI->TLSetState(*TL, currentInterval);

        // initialize TL status
        updateTLstate(*TL, "init", currentInterval);
    }

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl << endl;
}


void TrafficLightAdaptiveQueue::executeEachTimeStep(bool simulationDone)
{
    TrafficLightLowDelay::executeEachTimeStep(simulationDone);

    if(TLControlMode != TL_Adaptive_Time_Queue)
        return;

    intervalElapseTime += updateInterval;
}


void TrafficLightAdaptiveQueue::chooseNextInterval()
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
        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState("C", nextGreenInterval);
        intervalElapseTime = 0.0;
        intervalOffSet = greenInterval.front().greenTime;

        if(nextGreenIsNewCycle)
        {
            updateTLstate("C", "phaseEnd", nextGreenInterval, true);  // a new cycle
            nextGreenIsNewCycle = false;
        }
        else
            updateTLstate("C", "phaseEnd", nextGreenInterval);
    }
    else
        chooseNextGreenInterval();

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl << endl;
}


void TrafficLightAdaptiveQueue::chooseNextGreenInterval()
{
    // Remove current old phase:
    greenInterval.erase(greenInterval.begin());

    // Assign new green:
    if (greenInterval.empty())
    {
        calculatePhases("C");
        nextGreenIsNewCycle = true;
    }

    nextGreenInterval = greenInterval.front().greenString;

    // calculate 'next interval'
    std::string nextInterval = "";
    bool needYellowInterval = false;  // if we have at least one yellow interval
    for(unsigned int linkNumber = 0; linkNumber < currentInterval.size(); ++linkNumber)
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

        intervalElapseTime = 0.0;
        intervalOffSet =  yellowTime;

        // update TL status for this phase
        updateTLstate("C", "yellow");
    }
    // extend the current green interval
    else
    {
        intervalOffSet = greenInterval.front().greenTime;
        std::cout << ">>> Continue the last green interval." << endl << endl;
    }
}


// calculate all phases (up to 4)
void TrafficLightAdaptiveQueue::calculatePhases(std::string TLid)
{
    // batch of all non-conflicting movements, sorted by total queue size per batch
    std::priority_queue< batchMovementQueueEntry /*type of each element*/, std::vector<batchMovementQueueEntry> /*container*/, movementCompareQueue > batchMovementQueue;

    // clear the priority queue
    batchMovementQueue = std::priority_queue < batchMovementQueueEntry, std::vector<batchMovementQueueEntry>, movementCompareQueue >();

    for(unsigned int i = 0; i < allMovements.size(); ++i)  // row
    {
        int totalQueueRow = 0;  // total queue size for this batch of movements
        int oneCount = 0;

        // for each batch of movement get
        // - 'total queue size' : total queue size
        // - 'one count'        : # of allowed movements (except right turns)
        // - 'maxVehCount'      : max queue size
        int maxVehCount = 0;
        for(unsigned int j = 0; j < allMovements[i].size(); ++j)  // column (link number)
        {
            // if a right turn
            bool rightTurn = std::find(std::begin(rightTurns), std::end(rightTurns), j) != std::end(rightTurns);

            if(allMovements[i][j] == 1 && !rightTurn)
            {
                int queueSize = linkQueueSize[std::make_pair(TLid,j)];
                int queueSizeBounded = (maxQueueSize == -1) ? queueSize : std::min(maxQueueSize,queueSize);

                totalQueueRow = totalQueueRow + queueSizeBounded;
                oneCount++;
                maxVehCount = std::max(maxVehCount, queueSizeBounded);
            }
        }

        // add this batch of movements to priority_queue
        batchMovementQueueEntry *entry = new batchMovementQueueEntry(oneCount, totalQueueRow, maxVehCount, allMovements[i]);
        batchMovementQueue.push(*entry);
    }

    // copy batchMovementQueue to a vector for iteration:
    std::vector<batchMovementQueueEntry> batchMovementVector;
    while(!batchMovementQueue.empty())
    {
        batchMovementVector.push_back(batchMovementQueue.top());
        batchMovementQueue.pop();
    }

    // Select at most 4 phases for new cycle:
    while(!batchMovementVector.empty())
    {
        // Always select the first movement because it will be the best(?):
        std::vector<int> bestMovement = batchMovementVector.front().batchMovements;

        // calculate the next green interval.
        // right-turns are all permissive and are given 'g'
        std::string nextInterval = "";
        for(unsigned linkNumber = 0; linkNumber < bestMovement.size(); ++linkNumber)
        {
            // if a right turn
            bool rightTurn = std::find(std::begin(rightTurns), std::end(rightTurns), linkNumber) != std::end(rightTurns);

            if(bestMovement[linkNumber] == 0)
                nextInterval += 'r';
            else if(bestMovement[linkNumber] == 1 && rightTurn)
                nextInterval += 'g';
            else if(bestMovement[linkNumber] == 1 && !rightTurn)
                nextInterval += 'G';
        }

        // Avoid pushing "only permissive right turns" phase: todo: is this necessary?
        if (nextInterval == "grgrrgrgrrgrgrrgrgrrrrrr")
        {
            batchMovementVector.erase(batchMovementVector.begin());
            continue;
        }

        greenIntervalInfo *entry = new greenIntervalInfo(batchMovementVector.front().maxVehCount, 0.0, nextInterval);
        greenInterval.push_back(*entry);

        // Now delete these movements because they should never occur again:
        batchMovementVector.erase( std::remove_if(batchMovementVector.begin(), batchMovementVector.end(), served(bestMovement)), batchMovementVector.end() );
    }

    // throw error if cycle is more than 4 phases:
    if (greenInterval.size() > 4)
        error("cycle contains more than 4 phases!");

    // calculate maxVehCount in a cycle
    int maxVehCountCycle = 0;
    for (auto &i : greenInterval)
        maxVehCountCycle += i.maxVehCount;

    // If no vehicles in queue, then run each green interval with minGreenTime:
    if (maxVehCountCycle == 0)
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
    greenInterval.erase( std::remove_if(greenInterval.begin(), greenInterval.end(), noGreenTime), greenInterval.end() );
    int newSize = greenInterval.size();
    if(oldSize != newSize)
        std::cout << ">>> " << oldSize - newSize << " phase(s) removed due to zero queue size!" << endl << endl;

    // make sure the green splits are bounded
    for (auto &i : greenInterval)
        i.greenTime = std::min(std::max(i.greenTime, minGreenTime), maxGreenTime);

    std::cout << "Selected green intervals for this cycle: " << endl;
    for (auto &i : greenInterval)
        std::cout << "Movement " << i.greenString << " with maxVehCount of " << i.maxVehCount << " for " << i.greenTime << "s" << endl;

    std::cout << endl;
}

}

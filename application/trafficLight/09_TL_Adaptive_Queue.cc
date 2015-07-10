/****************************************************************************/
/// @file    TL_AdaptiveQueue.cc
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

#include <09_TL_Adaptive_Queue.h>
#include <boost/graph/adjacency_list.hpp>

std::vector<int> bestMovement;

namespace VENTOS {

Define_Module(VENTOS::TrafficLightAdaptiveQueue);


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

    std::cout << endl << "Adaptive-time with queue traffic signal control ..." << endl << endl;

    // get all non-conflicting movements in allMovements vector
    TrafficLightAllowedMoves::getMovements("C");

    // calculate phases at the beginning of the cycle
    calculatePhases("C");

    currentInterval = greenInterval.front().greenString;

    // Set initial settings:
    intervalElapseTime = 0;
    intervalOffSet = greenInterval.front().greenTime;

    scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);

    for (std::list<std::string>::iterator TL = TLList.begin(); TL != TLList.end(); ++TL)
    {
        TraCI->TLSetProgram(*TL, "adaptive-time");
        TraCI->TLSetState(*TL, currentInterval);
    }

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl;
}


void TrafficLightAdaptiveQueue::executeEachTimeStep(bool simulationDone)
{
    TrafficLightLowDelay::executeEachTimeStep(simulationDone);

    if(TLControlMode != TL_Adaptive_Time_Queue)
        return;

    intervalElapseTime += updateInterval;
}


bool served(batchMovementQueueEntry v)
{
    for (unsigned int i = 0; i < v.batchMovements.size(); i++)
    {
        // Ignore all permissive right turns since these are always green:
        if (i == 0 || i == 2 || i == 5 || i == 7 ||
                i == 10 || i == 12 || i == 15 || i == 17)
            continue;

        // Want to remove movements that have already been done:
        if (v.batchMovements[i] && bestMovement[i])
        {
            return true;
        }
    }
    return false;
}

bool noGreenTime(greenIntervalInfo v)
{
    if (v.greenTime == 0.0)
        return true;
    else
        return false;
}


// calculate all phases (up to 4)
void TrafficLightAdaptiveQueue::calculatePhases(std::string TLid)
{
    // clear the priority queue
    batchMovementQueue = std::priority_queue < batchMovementQueueEntry, std::vector<batchMovementQueueEntry>, movementCompareQueue >();

    // get which row has the highest queue length
    for(unsigned int i = 0; i < allMovements.size(); ++i)  // row
    {
        int totalQueueRow = 0;
        int oneCount = 0;

        for(unsigned int j = 0; j < allMovements[i].size(); ++j)  // column (link number)
        {
            if(allMovements[i][j] == 1)
            {
                totalQueueRow = totalQueueRow + linkQueueSize[std::make_pair(TLid,j)];
                oneCount++;
            }
        }

        // add this batch of movements to priority_queue
        batchMovementQueueEntry *entry = new batchMovementQueueEntry(oneCount, totalQueueRow, allMovements[i]);
        batchMovementQueue.push(*entry);
    }

    // Convert batchMovementQueue to a vector for iteration:
    std::vector<batchMovementQueueEntry> batchMovementVector;
    while(!batchMovementQueue.empty())
    {
        batchMovementVector.push_back(batchMovementQueue.top());
        batchMovementQueue.pop();
    }

    // Select at most 4 phases for new cycle:
    int queueTotal = 0;
    while(!batchMovementVector.empty())
    {
        // Always select the first movement because it will be the best(?):
        bestMovement = batchMovementVector.front().batchMovements;

        // Change all 1's to G, 0's to r:
        std::string nextInterval = "";
        for(int i : bestMovement) {
            if (i == 1)
                nextInterval += 'G';
            else
                nextInterval += 'r';
        }

        // Avoid pushing "only permissive right turns" phase:
        if (nextInterval == "GrGrrGrGrrGrGrrGrGrrrrrr")
        {
            batchMovementVector.erase(batchMovementVector.begin());
            continue;
        }

        greenIntervalInfo *entry = new greenIntervalInfo(batchMovementVector.front().totalQueue, 0.0, nextInterval);
        greenInterval.push_back(*entry);
        queueTotal += batchMovementVector.front().totalQueue;


        // Now delete these movements because they should never occur again:
        batchMovementVector.erase( std::remove_if(batchMovementVector.begin(), batchMovementVector.end(), served), batchMovementVector.end() );
    }

    // Now proportion cycle time to each of the phases:
    for (auto &i : greenInterval)
    {
        // If no vehicles in queue, then run each green interval with equal timing (?):
        if (queueTotal == 0)
            i.greenTime = 0.25 * cycleLength;
        else
            i.greenTime = (double(i.vehCount) / double(queueTotal)) * double(cycleLength);
    }

    // If no green time (0s) is given to an interval, then this queue is empty and useless:
    std::cout << "Phases in this cycle before removal: " << greenInterval.size() << endl;
    greenInterval.erase( std::remove_if(greenInterval.begin(), greenInterval.end(), noGreenTime), greenInterval.end() );
    std::cout << "Phases in this cycle after removal: " << greenInterval.size() << endl;

    for (auto &i : greenInterval)
    {
        std::cout << i.greenString << " for " << i.greenTime << "s" << endl;
    }

    // todo: Include error code if more than 4 phases:
    if (greenInterval.size() > 4)
    {

    }

    std::cout << endl;
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
    }
    else if (currentInterval == "red")
    {
        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState("C", nextGreenInterval);
        intervalElapseTime = 0.0;
        intervalOffSet = nextGreenDuration;
    }
    else
    {
        chooseNextGreenInterval();
    }

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl;
}


void TrafficLightAdaptiveQueue::chooseNextGreenInterval()
{
    // Remove current old phase:
    greenInterval.erase(greenInterval.begin());

    // Assign new green:
    if (greenInterval.empty())
        calculatePhases("C");

    nextGreenInterval = greenInterval.front().greenString;
    nextGreenDuration = greenInterval.front().greenTime;

    currentInterval = "yellow";

    // change all 'G/g' to 'y'
    std::string str = TraCI->TLGetState("C");
    std::string nextInterval = "";
    for(char& c : str) {
        if (c == 'G' || c == 'g')
            nextInterval += 'y';
        else
            nextInterval += c;
    }

    TraCI->TLSetState("C", nextInterval);

    intervalElapseTime = 0.0;
    intervalOffSet =  yellowTime;

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl << endl;
}

}

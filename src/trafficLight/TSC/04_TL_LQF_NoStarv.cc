/****************************************************************************/
/// @file    TL_LQF_NoStarv.cc
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

#include "04_TL_LQF_NoStarv.h"
#include <queue>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightLQF_NoStarv);

class sortedEntry_LQF
{
public:
    int oneCount;
    int totalQueue;
    int maxVehCount;
    std::vector<int> batchMovements;

    sortedEntry_LQF(int i1, int i2, int i3, std::vector<int> bm)
    {
        this->oneCount = i1;
        this->totalQueue = i2;
        this->maxVehCount = i3;
        batchMovements.swap(bm);
    }
};


class sortCompare_LQF
{
public:
    bool operator()(sortedEntry_LQF p1, sortedEntry_LQF p2)
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
struct served_LQF
{
public:
    served_LQF(std::vector<int> best)
{
        bestMovement.swap(best);
}

    bool operator () (const sortedEntry_LQF v)
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


TrafficLightLQF_NoStarv::~TrafficLightLQF_NoStarv()
{

}


void TrafficLightLQF_NoStarv::initialize(int stage)
{
    super::initialize(stage);

    if(TLControlMode != TL_LQF)
        return;

    if(stage == 0)
    {
        maxQueueSize = par("maxQueueSize").longValue();

        if(maxQueueSize <= 0 && maxQueueSize != -1)
            throw omnetpp::cRuntimeError("maxQueueSize value is set incorrectly!");

        nextGreenIsNewCycle = false;
        intervalChangeEVT = new omnetpp::cMessage("intervalChangeEVT", 1);

        // collect queue information
        measureIntersectionQueue = true;
    }
}


void TrafficLightLQF_NoStarv::finish()
{
    super::finish();
}


void TrafficLightLQF_NoStarv::handleMessage(omnetpp::cMessage *msg)
{
    super::handleMessage(msg);

    if(TLControlMode != TL_LQF)
        return;

    if (msg == intervalChangeEVT)
    {
        if(greenInterval.empty())
            calculatePhases("C");

        chooseNextInterval();

        if(intervalDuration <= 0)
            throw omnetpp::cRuntimeError("intervalDuration is <= 0");

        // Schedule next light change event:
        scheduleAt(omnetpp::simTime().dbl() + intervalDuration, intervalChangeEVT);
    }
}


void TrafficLightLQF_NoStarv::initialize_withTraCI()
{
    super::initialize_withTraCI();

    if(TLControlMode != TL_LQF)
        return;

    LOG_INFO << "\nLongest Queue traffic signal control ... \n" << std::flush;

    // get all non-conflicting movements in allMovements vector
    TrafficLightAllowedMoves::getMovements("C");

    // make sure allMovements vector is not empty
    ASSERT(!allMovements.empty());

    // calculate phases at the beginning of the cycle
    calculatePhases("C");

    // set initial settings:
    currentInterval = greenInterval.front().greenString;
    intervalDuration = greenInterval.front().greenTime;

    scheduleAt(omnetpp::simTime().dbl() + intervalDuration, intervalChangeEVT);

    for (auto &TL :TLList)
    {
        TraCI->TLSetProgram(TL, "adaptive-time");
        TraCI->TLSetState(TL, currentInterval);

        // initialize TL status
        updateTLstate(TL, "init", currentInterval);
    }

    LOG_DEBUG << boost::format("\n    SimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
    % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
}


void TrafficLightLQF_NoStarv::executeEachTimeStep()
{
    super::executeEachTimeStep();

    if(TLControlMode != TL_LQF)
        return;
}


void TrafficLightLQF_NoStarv::chooseNextInterval()
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
        if(nextGreenIsNewCycle)
        {
            updateTLstate("C", "phaseEnd", nextGreenInterval, true);  // a new cycle
            nextGreenIsNewCycle = false;
        }
        else
            updateTLstate("C", "phaseEnd", nextGreenInterval);

        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState("C", nextGreenInterval);
        intervalDuration = greenInterval.front().greenTime;
    }
    else
        chooseNextGreenInterval();

    LOG_DEBUG << boost::format("\n    SimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
    % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
}


void TrafficLightLQF_NoStarv::chooseNextGreenInterval()
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

        intervalDuration =  yellowTime;

        // update TL status for this phase
        updateTLstate("C", "yellow");
    }
    // extend the current green interval
    else
    {
        intervalDuration = greenInterval.front().greenTime;
        LOG_DEBUG << ">>> Continue the last green interval. \n\n" << std::flush;
    }
}


// calculate all phases (up to 4)
void TrafficLightLQF_NoStarv::calculatePhases(std::string TLid)
{
    LOG_DEBUG << "\n>>> New cycle calculation ... \n" << std::flush;

    if(LOG_ACTIVE(DEBUG_LOG_VAL))
    {
        LOG_DEBUG << "\n    Queue size per lane: ";
        for(auto& entry : laneQueueSize)
        {
            std::string lane = entry.first;
            int qSize = entry.second.second;
            if(qSize != 0)
                LOG_DEBUG << lane << " (" << qSize << ") | ";
        }
        LOG_DEBUG << "\n";
    }

    // batch of all non-conflicting movements, sorted by total queue size per batch
    std::priority_queue< sortedEntry_LQF /*type of each element*/, std::vector<sortedEntry_LQF> /*container*/, sortCompare_LQF > sortedMovements;
    // clear the priority queue
    sortedMovements = std::priority_queue < sortedEntry_LQF, std::vector<sortedEntry_LQF>, sortCompare_LQF >();

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
        sortedEntry_LQF *entry = new sortedEntry_LQF(oneCount, totalQueueRow, maxVehCount, allMovements[i]);
        sortedMovements.push(*entry);
    }

    // copy sortedMovements to a vector for iteration:
    std::vector<sortedEntry_LQF> batchMovementVector;
    while(!sortedMovements.empty())
    {
        batchMovementVector.push_back(sortedMovements.top());
        sortedMovements.pop();
    }

    // Select only the necessary phases for the new cycle:
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

        // todo: is this necessary?
        // Avoid pushing "only permissive right turns" phase:
        if (nextInterval == "grgrrgrgrrgrgrrgrgrrrrrr")
        {
            batchMovementVector.erase(batchMovementVector.begin());
            continue;
        }

        greenInterval_LQF *entry = new greenInterval_LQF(batchMovementVector.front().maxVehCount, 0.0, nextInterval);
        greenInterval.push_back(*entry);

        // Now delete these movements because they should never occur again:
        batchMovementVector.erase( std::remove_if(batchMovementVector.begin(), batchMovementVector.end(), served_LQF(bestMovement)), batchMovementVector.end() );
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
            [](const greenInterval_LQF v)
            {
        if (v.greenTime == 0.0)
            return true;
        else
            return false;
            }
    );
    greenInterval.erase( rme, greenInterval.end() );

    // throw error if cycle contains more than 4 phases:
    if (greenInterval.size() > 4)
        throw omnetpp::cRuntimeError("cycle contains %d phases which is more than 4!", greenInterval.size());

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
            LOG_DEBUG << boost::format("        Movement %1% with maxVehCount of %2% for %3%s \n") % i.greenString % i.maxVehCount % i.greenTime;

        LOG_FLUSH;
    }
}

}

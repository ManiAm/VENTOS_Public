/****************************************************************************/
/// @file    LQF_NoStarv.cc
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

#include <queue>

#include "trafficLight/TSC/04_LQF_NoStarv.h"

namespace VENTOS {

Define_Module(VENTOS::TrafficLightLQF_NoStarv);


TrafficLightLQF_NoStarv::~TrafficLightLQF_NoStarv()
{

}


void TrafficLightLQF_NoStarv::initialize(int stage)
{
    if(par("TLControlMode").intValue() == TL_LQF)
        par("record_intersectionQueue_stat") = true;

    super::initialize(stage);

    if(TLControlMode != TL_LQF)
        return;

    if(stage == 0)
    {
        nextGreenIsNewCycle = false;
        intervalChangeEVT = new omnetpp::cMessage("intervalChangeEVT", 1);
    }
}


void TrafficLightLQF_NoStarv::finish()
{
    super::finish();
}


void TrafficLightLQF_NoStarv::handleMessage(omnetpp::cMessage *msg)
{
    if (TLControlMode == TL_LQF && msg == intervalChangeEVT)
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


void TrafficLightLQF_NoStarv::initialize_withTraCI()
{
    super::initialize_withTraCI();

    if(TLControlMode != TL_LQF)
        return;

    LOG_INFO << "\nLongest Queue traffic signal control ... \n" << std::flush;

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

    // get all non-conflicting movements in allMovements vector
    allMovements = TrafficLightAllowedMoves::getMovements("C");

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

        // initialize TL status
        updateTLstate(TL, "init", currentInterval);
    }

    LOG_DEBUG << boost::format("\nSimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
    % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
}


void TrafficLightLQF_NoStarv::executeEachTimeStep()
{
    super::executeEachTimeStep();

    if(TLControlMode != TL_LQF)
        return;
}


void TrafficLightLQF_NoStarv::chooseNextInterval(std::string TLid)
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

    LOG_DEBUG << boost::format("\nSimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
    % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
}


void TrafficLightLQF_NoStarv::chooseNextGreenInterval(std::string TLid)
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
        TraCI->TLSetState(TLid, nextInterval);

        intervalDuration =  yellowTime;

        // update TL status for this phase
        updateTLstate(TLid, "yellow");
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
        LOG_DEBUG << "\n";
    }

    priorityQ sortedMovements;

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
            if(allMovements[i][j] == 1 && !isRightTurn(j))
            {
                std::string lane = link2Lane[std::make_pair(TLid,j)];
                auto queueSize = laneGetQueue(lane).queueSize;

                totalQueueRow += queueSize;
                oneCount++;
                maxVehCount = std::max(maxVehCount, queueSize);
            }
        }

        // add this batch of movements to priority_queue
        sortedEntry_t entry = {oneCount, totalQueueRow, maxVehCount, allMovements[i]};
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
        std::vector<int> bestMovement = batchMovementVector.front().batchMovements;

        // calculate the next green interval.
        // right-turns are all permissive and are given 'g'
        std::string nextInterval = "";
        for(unsigned linkNumber = 0; linkNumber < bestMovement.size(); ++linkNumber)
        {
            if(bestMovement[linkNumber] == 0)
                nextInterval += 'r';
            else if(bestMovement[linkNumber] == 1)
            {
                if(isRightTurn(linkNumber))
                    nextInterval += 'g';
                else
                    nextInterval += 'G';
            }
        }

        // todo: is this necessary?
        // Avoid pushing "only permissive right turns" phase:
        if (nextInterval == "grgrrgrgrrgrgrrgrgrrrrrr")
        {
            batchMovementVector.erase(batchMovementVector.begin());
            continue;
        }

        greenIntervalEntry_t entry = {batchMovementVector.front().maxVehCount, 0.0, nextInterval};
        greenInterval.push_back(entry);

        // Now delete these movements because they should never occur again:
        auto new_end = std::remove_if(batchMovementVector.begin(), batchMovementVector.end(), [bestMovement](const sortedEntry_t v) {
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
    auto rme = std::remove_if(greenInterval.begin(), greenInterval.end(), [](const greenIntervalEntry_t v) {
        if (v.greenTime == 0.0)
            return true;
        else
            return false; });
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

    LOG_DEBUG << "------------------------------------------------------------------------------- \n" << std::flush;
}

}

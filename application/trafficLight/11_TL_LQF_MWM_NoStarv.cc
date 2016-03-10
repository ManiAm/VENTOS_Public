/****************************************************************************/
/// @file    TL_LQF_MWM_NoStarv.cc
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

#include <11_TL_LQF_MWM_NoStarv.h>
#include <queue>

namespace VENTOS {

Define_Module(VENTOS::TrafficLight_LQF_MWM_NoStarv);

class sortedEntryLQF
{
public:
    double totalWeight;
    int oneCount;
    int maxVehCount;
    std::string phase;

    sortedEntryLQF(double d1, int i1, int i2, std::string p)
    {
        this->totalWeight = d1;
        this->oneCount = i1;
        this->maxVehCount = i2;
        this->phase = p;
    }
};


class sortCompareLQF
{
public:
    bool operator()(sortedEntryLQF p1, sortedEntryLQF p2)
    {
        if( p1.totalWeight < p2.totalWeight )
            return true;
        else if( p1.totalWeight == p2.totalWeight && p1.oneCount < p2.oneCount)
            return true;
        else
            return false;
    }
};


// using a 'functor' rather than a 'function'
// Reason: to be able to pass an additional argument (bestMovement) to predicate
struct servedLQF
{
public:
    servedLQF(std::string str)
{
        this->thisPhase = str;
}

    bool operator () (const sortedEntryLQF v)
    {
        std::string phase = v.phase;

        for(unsigned int i = 0; i < phase.size(); i++)
        {
            if (phase[i] == 'G' && thisPhase[i] == 'G')
                return true;
        }

        return false;
    }

private:
    std::string thisPhase;
};


TrafficLight_LQF_MWM_NoStarv::~TrafficLight_LQF_MWM_NoStarv()
{

}


void TrafficLight_LQF_MWM_NoStarv::initialize(int stage)
{
    TrafficLight_LQF_MWM::initialize(stage);

    if(TLControlMode != TL_LQF_MWM)
        return;

    if(stage == 0)
    {
        nextGreenIsNewCycle = false;
        ChangeEvt = new cMessage("ChangeEvt", 1);
    }
}


void TrafficLight_LQF_MWM_NoStarv::finish()
{
    TrafficLight_LQF_MWM::finish();
}


void TrafficLight_LQF_MWM_NoStarv::handleMessage(cMessage *msg)
{
    TrafficLight_LQF_MWM::handleMessage(msg);

    if(TLControlMode != TL_LQF_MWM)
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


void TrafficLight_LQF_MWM_NoStarv::executeFirstTimeStep()
{
    // call parent
    TrafficLight_LQF_MWM::executeFirstTimeStep();

    if(TLControlMode != TL_LQF_MWM)
        return;

    std::cout << endl << "Multi-class LQF-MWM traffic signal control ..." << endl << endl;

    // find the RSU module that controls this TL
    findRSU("C");

    // make sure RSUptr is pointing to our corresponding RSU
    ASSERT(RSUptr);

    // turn on active detection on this RSU
    RSUptr->par("activeDetection") = true;

    // calculate phases at the beginning of the cycle
    calculatePhases("C");

    // set initial settings:
    currentInterval = greenInterval.front().greenString;
    intervalOffSet = greenInterval.front().greenTime;
    intervalElapseTime = 0;

    scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);

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
        sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
        std::cout << endl << buff << endl << endl;
        std::cout.flush();
    }
}


void TrafficLight_LQF_MWM_NoStarv::executeEachTimeStep()
{
    // call parent
    TrafficLight_LQF_MWM::executeEachTimeStep();

    if(TLControlMode != TL_LQF_MWM)
        return;

    intervalElapseTime += updateInterval;
}


void TrafficLight_LQF_MWM_NoStarv::chooseNextInterval()
{
    if (currentInterval == "yellow")
    {
        currentInterval = "red";

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
        intervalElapseTime = 0.0;
        intervalOffSet = greenInterval.front().greenTime;
    }
    else
        chooseNextGreenInterval();

    if(ev.isGUI() && debugLevel > 0)
    {
        char buff[300];
        sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
        std::cout << buff << endl << endl;
        std::cout.flush();
    }
}


void TrafficLight_LQF_MWM_NoStarv::chooseNextGreenInterval()
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
    for(unsigned int linkNumber = 0; linkNumber < currentInterval.size(); ++linkNumber)
    {
        if( (currentInterval[linkNumber] == 'G' || currentInterval[linkNumber] == 'g') && nextGreenInterval[linkNumber] == 'r')
            nextInterval += 'y';
        else
            nextInterval += currentInterval[linkNumber];
    }

    currentInterval = "yellow";
    TraCI->TLSetState("C", nextInterval);

    intervalElapseTime = 0.0;
    intervalOffSet =  yellowTime;

    // update TL status for this phase
    updateTLstate("C", "yellow");
}


void TrafficLight_LQF_MWM_NoStarv::calculatePhases(std::string TLid)
{
    std::map<std::string, laneInfoEntry> laneInfo = RSUptr->laneInfo;

    if(laneInfo.empty())
        error("LaneInfo is empty! Is active detection on in %s ?", RSUptr->getFullName());

    // batch of all non-conflicting movements, sorted by total weight + oneCount per batch
    std::priority_queue< sortedEntryLQF /*type of each element*/, std::vector<sortedEntryLQF> /*container*/, sortCompareLQF > sortedMovements;

    // clear the priority queue
    sortedMovements = std::priority_queue < sortedEntryLQF, std::vector<sortedEntryLQF>, sortCompareLQF >();

    for(std::string phase : phases)
    {
        double totalWeight = 0;  // total weight for each batch
        int oneCount = 0;
        int maxVehCount = 0;

        for(unsigned int linkNumber = 0; linkNumber < phase.size(); ++linkNumber)
        {
            int vehCount = 0;
            if(phase[linkNumber] == 'G')
            {
                bool rightTurn = std::find(std::begin(rightTurns), std::end(rightTurns), linkNumber) != std::end(rightTurns);
                // ignore this link if right turn
                if(!rightTurn)
                {
                    // get the corresponding lane for this link
                    auto itt = linkLane.find(std::make_pair(TLid,linkNumber));
                    if(itt == linkLane.end())
                        error("linkNumber %s is not found in TL %s", linkNumber, TLid.c_str());
                    std::string lane = itt->second;

                    // find this lane in laneInfo
                    auto res = laneInfo.find(lane);
                    if(res == laneInfo.end())
                        error("Can not find lane %s in laneInfo!", lane.c_str());

                    // get all queued vehicles on this lane
                    auto vehicles = (*res).second.allVehicles;

                    // for each vehicle
                    for(auto& entry : vehicles)
                    {
                        // we only care about waiting vehicles on this lane
                        if(entry.second.vehStatus == VEH_STATUS_Waiting)
                        {
                            vehCount++;

                            std::string vID = entry.first;
                            std::string vType = entry.second.vehType;

                            // total weight of entities on this lane
                            auto loc = classWeight.find(vType);
                            if(loc == classWeight.end())
                                error("entity %s with type %s does not have a weight in classWeight map!", vID.c_str(), vType.c_str());
                            totalWeight += loc->second;
                        }
                    }
                }

                // number of movements in this phase (including right turns)
                oneCount++;
            }

            maxVehCount = std::max(maxVehCount, vehCount);
        }

        // add this batch of movements to priority_queue
        sortedEntryLQF *entry = new sortedEntryLQF(totalWeight, oneCount, maxVehCount, phase);
        sortedMovements.push(*entry);
    }

    // copy sortedMovements to a vector for iteration:
    std::vector<sortedEntryLQF> batchMovementVector;
    while(!sortedMovements.empty())
    {
        batchMovementVector.push_back(sortedMovements.top());
        sortedMovements.pop();
    }

    // Select only the necessary phases for the new cycle:
    while(!batchMovementVector.empty())
    {
        // Always select the first movement because it will be the best(?):
        sortedEntryLQF entry = batchMovementVector.front();
        std::string nextInterval = entry.phase;

        greenIntervalInfo_LQF *entry2 = new greenIntervalInfo_LQF(entry.maxVehCount, entry.totalWeight, entry.oneCount, 0.0, entry.phase);
        greenInterval.push_back(*entry2);

        // Now delete these movements because they should never occur again:
        batchMovementVector.erase( std::remove_if(batchMovementVector.begin(), batchMovementVector.end(), servedLQF(nextInterval)), batchMovementVector.end() );
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
            [](const greenIntervalInfo_LQF v)
            {
        if (v.greenTime == 0.0)
            return true;
        else
            return false;
            }
    );
    greenInterval.erase( rme, greenInterval.end() );
    int newSize = greenInterval.size();
    if(ev.isGUI() && debugLevel > 1 && oldSize != newSize)
    {
        std::cout << ">>> " << oldSize - newSize << " phase(s) removed due to zero queue size!" << endl << endl;
        std::cout.flush();
    }

    // make sure the green splits are bounded
    for (auto &i : greenInterval)
        i.greenTime = std::min(std::max(i.greenTime, minGreenTime), maxGreenTime);

    if(ev.isGUI() && debugLevel > 1)
    {
        std::cout << "Selected green intervals for this cycle: " << endl;
        for (auto &i : greenInterval)
            std::cout << "movement: " << i.greenString
            << ", maxVehCount= " << i.maxVehCount
            << ", totalWeight= " << i.totalWeight
            << ", oneCount= " << i.oneCount
            << ", green= " << i.greenTime << "s" << endl;

        std::cout << endl;
        std::cout.flush();
    }
}

}

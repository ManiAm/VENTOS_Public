/****************************************************************************/
/// @file    13_TL_FMSC.cc
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

#include <13_TL_FMSC.h>
#include <queue>

namespace VENTOS {

Define_Module(VENTOS::TrafficLight_FMSC);

class sortedEntryFMSC
{
public:
    int oneCount;
    int maxVehCount;
    double totalDelay;
    double maxWeight;
    std::vector<int> batchMovements;

    sortedEntryFMSC(double d1, double d2, int i1, int i2, std::vector<int> bm)
    {
        this->maxWeight = d1;
        this->totalDelay = d2;
        this->oneCount = i1;
        this->maxVehCount = i2;
        batchMovements.swap(bm);
    }
};


class sortCompareFMSC
{
public:
    bool operator()(sortedEntryFMSC p1, sortedEntryFMSC p2)
    {
        if(p1.maxWeight < p2.maxWeight)
            return true;
        else if(p1.maxWeight == p2.maxWeight && p1.totalDelay < p2.totalDelay)
            return true;
        else if(p1.maxWeight == p2.maxWeight && p1.totalDelay == p2.totalDelay && p1.oneCount < p2.oneCount)
            return true;
        else
            return false;
    }
};


TrafficLight_FMSC::~TrafficLight_FMSC()
{

}


void TrafficLight_FMSC::initialize(int stage)
{
    TrafficLight_LQF_MWM_Cycle::initialize(stage);

    if(TLControlMode != TL_FMSC)
        return;

    if(stage == 0)
    {
        intervalChangeEVT = new cMessage("intervalChangeEVT", 1);
    }
}


void TrafficLight_FMSC::finish()
{
    TrafficLight_LQF_MWM_Cycle::finish();
}


void TrafficLight_FMSC::handleMessage(cMessage *msg)
{
    TrafficLight_LQF_MWM_Cycle::handleMessage(msg);

    if(TLControlMode != TL_FMSC)
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


void TrafficLight_FMSC::executeFirstTimeStep()
{
    // call parent
    TrafficLight_LQF_MWM_Cycle::executeFirstTimeStep();

    if(TLControlMode != TL_FMSC)
        return;

    std::cout << endl << "FMSC traffic signal control ..." << endl << endl;

    // find the RSU module that controls this TL
    findRSU("C");

    // make sure RSUptr is pointing to our corresponding RSU
    ASSERT(RSUptr);

    // turn on active detection on this RSU
    RSUptr->par("activeDetection") = true;

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


void TrafficLight_FMSC::executeEachTimeStep()
{
    // call parent
    TrafficLight_LQF_MWM_Cycle::executeEachTimeStep();

    if(TLControlMode != TL_FMSC)
        return;
}


void TrafficLight_FMSC::chooseNextInterval()
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
        intervalDuration = redTime;

        // update TL status for this phase
        updateTLstate("C", "red");
    }
    else if (currentInterval == "red")
    {
        // update TL status for this phase
        if(nextGreenInterval == firstGreen["C"])
            updateTLstate("C", "phaseEnd", nextGreenInterval, true);  //todo: notion of cycle?
        else
            updateTLstate("C", "phaseEnd", nextGreenInterval);

        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState("C", nextGreenInterval);
        intervalDuration = minGreenTime;
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


void TrafficLight_FMSC::chooseNextGreenInterval()
{
    std::map<std::string, laneInfoEntry> laneInfo = RSUptr->laneInfo;

    if(laneInfo.empty())
        error("LaneInfo is empty! Is active detection on in %s ?", RSUptr->getFullName());

    // batch of all non-conflicting movements, sorted by maxWeight + total delay + oneCount
    std::priority_queue< sortedEntryFMSC /*type of each element*/, std::vector<sortedEntryFMSC> /*container*/, sortCompareFMSC > sortedMovements;

    // clear the priority queue
    sortedMovements = std::priority_queue < sortedEntryFMSC, std::vector<sortedEntryFMSC>, sortCompareFMSC >();

    // calculate delay, max weight for each movement combination
    for(unsigned int i = 0; i < allMovements.size(); ++i)  // row
    {
        double maxWeight = 0;  // maximum weight for this batch
        double totalDelay = 0;
        int oneCount = 0;
        int maxVehCount = 0;

        for(unsigned int linkNumber = 0; linkNumber < allMovements[i].size(); ++linkNumber)  // column (link number)
        {
            int vehCount = 0;
            if(allMovements[i][linkNumber] == 1)
            {
                bool rightTurn = std::find(std::begin(rightTurns), std::end(rightTurns), linkNumber) != std::end(rightTurns);
                // ignore this link if right turn
                if(!rightTurn)
                {
                    // get the corresponding lane for this link
                    auto itt = linkToLane.find(std::make_pair("C",linkNumber));
                    if(itt == linkToLane.end())
                        error("linkNumber %s is not found in TL %s", linkNumber, "C");
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

                            // max weight of entities on this lane
                            auto loc = classWeight.find(vType);
                            if(loc == classWeight.end())
                                error("entity %s with type %s does not have a weight in classWeight map!", vID.c_str(), vType.c_str());
                            maxWeight = std::max(maxWeight, loc->second);

                            // total delay in this lane
                            auto locc = laneDelay[lane].find(vID);
                            if(locc != laneDelay[lane].end())
                                totalDelay += locc->second;
                        }
                    }
                }

                // number of movements in this phase (including right turns)
                oneCount++;
            }

            maxVehCount = std::max(maxVehCount, vehCount);
        }

        // add this batch of movements to priority_queue
        sortedEntryFMSC *entry = new sortedEntryFMSC(maxWeight, totalDelay, oneCount, maxVehCount, allMovements[i]);
        sortedMovements.push(*entry);
    }

    // get the movement batch with the highest weight + delay + oneCount
    sortedEntryFMSC entry = sortedMovements.top();
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

    // allocate enough green time to move all vehicles
    int maxVehCount = entry.maxVehCount;
    if(ev.isGUI() && debugLevel > 1)
    {
        std::cout << "Maximum of " << maxVehCount << " vehicle(s) are waiting. ";
        std::cout.flush();
    }
    double greenTime = (double)maxVehCount * (minGreenTime / 5.);
    nextGreenTime = std::min(std::max(greenTime, minGreenTime), maxGreenTime);      // bound green time
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

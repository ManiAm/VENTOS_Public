/****************************************************************************/
/// @file    TL_OJF_MWM.cc
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

#include <11_TL_OJF_MWM.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLight_OJF_MWM);

class sortedEntryOJF
{
public:
    double maxWeight;
    double totalDelay;
    int oneCount;
    int maxVehCount;
    std::vector<int> batchMovements;

    sortedEntryOJF(double d1, double d2, int i1, int i2, std::vector<int> bm)
    {
        this->maxWeight = d1;
        this->totalDelay = d2;
        this->oneCount = i1;
        this->maxVehCount = i2;
        batchMovements.swap(bm);
    }
};


class sortCompareOJF
{
public:
    bool operator()(sortedEntryOJF p1, sortedEntryOJF p2)
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


TrafficLight_OJF_MWM::~TrafficLight_OJF_MWM()
{

}


void TrafficLight_OJF_MWM::initialize(int stage)
{
    TrafficLight_LQF_MWM::initialize(stage);

    if(TLControlMode != TL_OJF_MWM)
        return;

    if(stage == 0)
    {
        // turn on active detection
        activeDetection = true;
        this->par("activeDetection") = true;

        ChangeEvt = new cMessage("ChangeEvt", 1);
    }
}


void TrafficLight_OJF_MWM::finish()
{
    TrafficLight_LQF_MWM::finish();
}


void TrafficLight_OJF_MWM::handleMessage(cMessage *msg)
{
    TrafficLight_LQF_MWM::handleMessage(msg);

    if(TLControlMode != TL_OJF_MWM)
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


void TrafficLight_OJF_MWM::executeFirstTimeStep()
{
    // call parent
    TrafficLight_LQF_MWM::executeFirstTimeStep();

    if(TLControlMode != TL_OJF_MWM)
        return;

    std::cout << endl << "Multi-class OJF-MWM traffic signal control ..." << endl << endl;

    // set initial values
    currentInterval = phase1_5;
    intervalOffSet = minGreenTime;
    intervalElapseTime = 0;

    scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);

    // get all non-conflicting movements in allMovements vector
    TrafficLightAllowedMoves::getMovements("C");

    for (auto &TL : TLList)
    {
        TraCI->TLSetProgram(TL, "adaptive-time");
        TraCI->TLSetState(TL, currentInterval);

        firstGreen[TL] = currentInterval;

        // initialize TL status
        updateTLstate(TL, "init", currentInterval);
    }

    // make sure RSUptr is pointing to our corresponding RSU
    ASSERT(RSUptr);

    if(debugLevel > 0)
    {
    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl << endl;
    }
}


void TrafficLight_OJF_MWM::executeEachTimeStep()
{
    // call parent
    TrafficLight_LQF_MWM::executeEachTimeStep();

    if(TLControlMode != TL_OJF_MWM)
        return;

    intervalElapseTime += updateInterval;
}


void TrafficLight_OJF_MWM::chooseNextInterval()
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
        // update TL status for this phase
        if(nextGreenInterval == firstGreen["C"])
            updateTLstate("C", "phaseEnd", nextGreenInterval, true);  //todo: notion of cycle?
        else
            updateTLstate("C", "phaseEnd", nextGreenInterval);

        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState("C", nextGreenInterval);
        intervalElapseTime = 0.0;
        intervalOffSet = minGreenTime;
    }
    else
        chooseNextGreenInterval();

    if(debugLevel > 0)
    {
    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl << endl;
    }
}


void TrafficLight_OJF_MWM::chooseNextGreenInterval()
{
    std::map<std::string, laneInfoEntry> laneInfo = RSUptr->laneInfo;

    // batch of all non-conflicting movements, sorted by maxWeight + total delay + oneCount
    std::priority_queue< sortedEntryOJF /*type of each element*/, std::vector<sortedEntryOJF> /*container*/, sortCompareOJF > sortedMovements;

    // clear the priority queue
    sortedMovements = std::priority_queue < sortedEntryOJF, std::vector<sortedEntryOJF>, sortCompareOJF >();

    // get which row has the highest delay
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
                    std::string lane = linkLane[std::make_pair("C",linkNumber)];

                    // find this lane in laneInfo
                    auto res = laneInfo.find(lane);

                    // get all queued vehicles on this lane
                    std::map<std::string /*vehicle id*/, queuedVehiclesEntry> queuedVehicles = (*res).second.queuedVehicles;

                    vehCount = queuedVehicles.size();

                    // for each vehicle
                    for(auto& entry : queuedVehicles)
                    {
                        std::string vID = entry.first;
                        std::string vType = entry.second.vehicleType;

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

                // including right turns
                oneCount++;
            }

            maxVehCount = std::max(maxVehCount, vehCount);
        }

        // add this batch of movements to priority_queue
        sortedEntryOJF *entry = new sortedEntryOJF(maxWeight, totalDelay, oneCount, maxVehCount, allMovements[i]);
        sortedMovements.push(*entry);
    }

    // get the movement batch with the highest weight + delay + oneCount
    sortedEntryOJF entry = sortedMovements.top();
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
    if(debugLevel > 1)
        std::cout << "Maximum of " << maxVehCount << " vehicle(s) are waiting. ";
    double greenTime = (double)maxVehCount * (minGreenTime / 5.);
    nextGreenTime = std::min(std::max(greenTime, minGreenTime), maxGreenTime);      // bound green time
    if(debugLevel > 1)
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
        if(debugLevel > 0)
            std::cout << ">>> Continue the last green interval." << endl << endl;
    }
}

}

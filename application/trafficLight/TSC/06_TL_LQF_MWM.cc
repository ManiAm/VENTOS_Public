/****************************************************************************/
/// @file    TL_LQF_MWM.cc
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

#include <06_TL_LQF_MWM.h>
#include <queue>

namespace VENTOS {

Define_Module(VENTOS::TrafficLight_LQF_MWM);

class sortedEntryLQF
{
public:
    int oneCount;
    int maxVehCount;
    double totalWeight;
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


TrafficLight_LQF_MWM::~TrafficLight_LQF_MWM()
{

}


void TrafficLight_LQF_MWM::initialize(int stage)
{
    TrafficLightOJF::initialize(stage);

    if(TLControlMode != TL_LQF_MWM)
        return;

    if(stage == 0)
    {
        intervalChangeEVT = new cMessage("intervalChangeEVT", 1);
    }
}


void TrafficLight_LQF_MWM::finish()
{
    TrafficLightOJF::finish();
}


void TrafficLight_LQF_MWM::handleMessage(cMessage *msg)
{
    TrafficLightOJF::handleMessage(msg);

    if(TLControlMode != TL_LQF_MWM)
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


void TrafficLight_LQF_MWM::executeFirstTimeStep()
{
    // call parent
    TrafficLightOJF::executeFirstTimeStep();

    if(TLControlMode != TL_LQF_MWM)
        return;

    std::cout << endl << "Multi-class LQF-MWM traffic signal control ..." << endl << endl;

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
        std::cout << endl << buff << endl << endl;
        std::cout.flush();
    }
}


void TrafficLight_LQF_MWM::executeEachTimeStep()
{
    // call parent
    TrafficLightOJF::executeEachTimeStep();

    if(TLControlMode != TL_LQF_MWM)
        return;
}


void TrafficLight_LQF_MWM::chooseNextInterval()
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


void TrafficLight_LQF_MWM::chooseNextGreenInterval()
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

    // get the movement batch with the highest weight + delay + oneCount
    sortedEntryLQF entry = sortedMovements.top();
    // this will be the next green interval
    nextGreenInterval = entry.phase;

    // calculate 'next interval'
    std::string nextInterval = "";
    bool needYellowInterval = false;  // if we have at least one yellow interval
    for(unsigned int linkNumber = 0; linkNumber < nextGreenInterval.size(); ++linkNumber)
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
    double greenTime = (double)maxVehCount * (minGreenTime / 5.);
    nextGreenTime = std::min(std::max(greenTime, minGreenTime), maxGreenTime);  // bound green time

    if(ev.isGUI() && debugLevel > 1)
    {
        printf("The following phase has the highest totalWeight out of %lu phases: \n", phases.size());
        printf("phase= %s", entry.phase.c_str());
        printf(", maxVehCount= %d", entry.maxVehCount);
        printf(", totalWeight= %0.2f", entry.totalWeight);
        printf(", oneCount= %d", entry.oneCount);
        printf(", green= %0.2fs \n", nextGreenTime);

        std::cout << endl;
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

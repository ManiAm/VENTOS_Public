/****************************************************************************/
/// @file    TL_LQF_MWM_Aging.cc
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

#include <07_TL_LQF_MWM_Aging.h>

namespace VENTOS {

class sortedEntryDelayLQF : public sortedEntryLQF
{
public:
    double maxDelay;

    sortedEntryDelayLQF(double d1, int i1, int i2, double d2, std::string p) : sortedEntryLQF(d1, i1, i2, p)
    {
        this->maxDelay = d2;
    }
};


class CompareDelay
{
public:
    bool operator()(sortedEntryDelayLQF n1, sortedEntryDelayLQF n2)
    {
        if (n1.maxDelay < n2.maxDelay)
            return true;
        else if (n1.maxDelay == n2.maxDelay && n1.totalWeight < n2.totalWeight)
            return true;
        else if (n1.maxDelay == n2.maxDelay && n1.totalWeight == n2.totalWeight && n1.oneCount < n2.oneCount)
            return true;
        else
            return false;
    }
};


Define_Module(VENTOS::TrafficLight_LQF_MWM_Aging);

TrafficLight_LQF_MWM_Aging::~TrafficLight_LQF_MWM_Aging()
{

}


void TrafficLight_LQF_MWM_Aging::initialize(int stage)
{
    super::initialize(stage);

    if(TLControlMode != TL_LQF_MWM_Aging)
        return;

    if(stage == 0)
    {
        intervalChangeEVT = new omnetpp::cMessage("intervalChangeEVT", 1);
    }
}


void TrafficLight_LQF_MWM_Aging::finish()
{
    super::finish();
}


void TrafficLight_LQF_MWM_Aging::handleMessage(omnetpp::cMessage *msg)
{
    super::handleMessage(msg);

    if(TLControlMode != TL_LQF_MWM_Aging)
        return;

    if (msg == intervalChangeEVT)
    {
        chooseNextInterval();

        if(intervalDuration <= 0)
            error("intervalDuration is <= 0");

        // Schedule next light change event:
        scheduleAt(omnetpp::simTime().dbl() + intervalDuration, intervalChangeEVT);
    }
}


void TrafficLight_LQF_MWM_Aging::initialize_withTraCI()
{
    // call parent
    super::initialize_withTraCI();

    if(TLControlMode != TL_LQF_MWM_Aging)
        return;

    std::cout << std::endl << "Multi-class LQF-MWM-Aging traffic signal control ..." << std::endl << std::endl;

    // find the RSU module that controls this TL
    findRSU("C");

    // make sure RSUptr is pointing to our corresponding RSU
    ASSERT(RSUptr);

    // turn on active detection on this RSU
    RSUptr->par("activeDetection") = true;

    // set initial values
    currentInterval = phase1_5;
    intervalDuration = minGreenTime;

    scheduleAt(omnetpp::simTime().dbl() + intervalDuration, intervalChangeEVT);

    for (auto &TL : TLList)
    {
        TraCI->TLSetProgram(TL, "adaptive-time");
        TraCI->TLSetState(TL, currentInterval);

        firstGreen[TL] = currentInterval;

        // initialize TL status
        updateTLstate(TL, "init", currentInterval);
    }

    if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 0)
    {
        char buff[300];
        sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", omnetpp::simTime().dbl(), currentInterval.c_str(), omnetpp::simTime().dbl(), omnetpp::simTime().dbl() + intervalDuration);
        std::cout << std::endl << buff << std::endl << std::endl;
        std::cout.flush();
    }
}


void TrafficLight_LQF_MWM_Aging::executeEachTimeStep()
{
    // call parent
    super::executeEachTimeStep();

    if(TLControlMode != TL_LQF_MWM_Aging)
        return;
}


void TrafficLight_LQF_MWM_Aging::chooseNextInterval()
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

    if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 0)
    {
        char buff[300];
        sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", omnetpp::simTime().dbl(), currentInterval.c_str(), omnetpp::simTime().dbl(), omnetpp::simTime().dbl() + intervalDuration);
        std::cout << buff << std::endl << std::endl;
        std::cout.flush();
    }
}


void TrafficLight_LQF_MWM_Aging::chooseNextGreenInterval()
{
    // get all incoming lanes for this TL only
    std::map<std::string /*lane*/, laneInfoEntry> laneInfo = RSUptr->laneInfo;

    if(laneInfo.empty())
        error("LaneInfo is empty! Is active detection on in %s ?", RSUptr->getFullName());

    // batch of all non-conflicting movements, sorted by total weight + oneCount per batch
    std::priority_queue< sortedEntryLQF /*type of each element*/, std::vector<sortedEntryLQF> /*container*/, sortCompareLQF > sortedMovements;

    // clear the priority queue
    sortedMovements = std::priority_queue < sortedEntryLQF, std::vector<sortedEntryLQF>, sortCompareLQF >();

    // maximum delay in each phase
    std::priority_queue< sortedEntryDelayLQF, std::vector<sortedEntryDelayLQF>, CompareDelay > maxDelayPerPhase;

    // clear the priority queue
    maxDelayPerPhase = std::priority_queue< sortedEntryDelayLQF, std::vector<sortedEntryDelayLQF>, CompareDelay > ();

    for(std::string phase : phases)
    {
        double totalWeight = 0;  // total weight for each batch
        int oneCount = 0;
        int maxVehCount = 0;
        double maxDelay = 0;

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

                            // max delay in this movement
                            auto ii = vehDelay.find(vID);
                            if(ii != vehDelay.end())
                                maxDelay = std::max(maxDelay, ii->second.accumDelay);  // todo: should we consider waiting time only?
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

        // add this batch of movements to priority_queue
        sortedEntryDelayLQF *entry2 = new sortedEntryDelayLQF(totalWeight, oneCount, maxVehCount, maxDelay, phase);
        maxDelayPerPhase.push(*entry2);
    }

    // get the movement batch with the highest weight + delay + oneCount
    sortedEntryLQF bestChoice = sortedMovements.top();

    // bestChoice selects the phase with the highest total weight.
    // We need to check the max delay in each phase too!
    sortedEntryLQF finalChoice = bestChoice;
    bool found = false;
    while(!maxDelayPerPhase.empty())
    {
        double maxDelay = maxDelayPerPhase.top().maxDelay;
        // do we exceed 'max delay' in a phase?
        if(!found && maxDelay + yellowTime + redTime > 20)
        {
            finalChoice = maxDelayPerPhase.top();
            found = true;
        }

        if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 1)
        {
            printf("Max delay in phase %s is %0.2f \n", maxDelayPerPhase.top().phase.c_str(), maxDelay);

            // todo
            // max delay in this movement
            auto ii = vehDelay.find("bike1");
            if(ii != vehDelay.end())
                std::cout << "bike delay is " << ii->second.accumDelay << std::endl;
        }

        maxDelayPerPhase.pop();
    }

    // allocate enough green time to move all vehicles
    int maxVehCount = finalChoice.maxVehCount;
    double greenTime = (double)maxVehCount * (minGreenTime / 5.);
    nextGreenTime = std::min(std::max(greenTime, minGreenTime), maxGreenTime);  // bound green time

    if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 1)
    {
        printf("\n");

        if(bestChoice.phase == finalChoice.phase)
        {
            printf("The following phase has the highest totalWeight out of %lu phases: \n", phases.size());
        }
        else
        {
            printf("Phase %s will not be scheduled! \n", bestChoice.phase.c_str());
            printf("Max delay in phase %s exceeds %ds \n", finalChoice.phase.c_str(), 20);
        }

        printf("phase= %s", finalChoice.phase.c_str());
        printf(", maxVehCount= %d", finalChoice.maxVehCount);
        printf(", totalWeight= %0.2f", finalChoice.totalWeight);
        printf(", oneCount= %d", finalChoice.oneCount);
        printf(", green= %0.2fs \n", nextGreenTime);

        std::cout << std::endl;
        std::cout.flush();
    }

    // this will be the next green interval
    nextGreenInterval = finalChoice.phase;

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
        if(omnetpp::cSimulation::getActiveEnvir()->isGUI() && debugLevel > 0)
        {
            std::cout << ">>> Continue the last green interval." << std::endl << std::endl;
            std::cout.flush();
        }
    }
}

}

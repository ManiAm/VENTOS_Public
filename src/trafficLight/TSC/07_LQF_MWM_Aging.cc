/****************************************************************************/
/// @file    LQF_MWM_Aging.cc
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

#include "trafficLight/TSC/07_LQF_MWM_Aging.h"

namespace VENTOS {

Define_Module(VENTOS::TrafficLight_LQF_MWM_Aging);

TrafficLight_LQF_MWM_Aging::~TrafficLight_LQF_MWM_Aging()
{

}


void TrafficLight_LQF_MWM_Aging::initialize(int stage)
{
    if(par("TLControlMode").intValue() == TL_LQF_MWM_Aging)
    {
        par("record_intersectionQueue_stat") = true;
        par("record_intersectionDelay_stat") = true;
    }

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
    if (TLControlMode == TL_LQF_MWM_Aging && msg == intervalChangeEVT)
    {
        chooseNextInterval("C");

        if(intervalDuration <= 0)
            throw omnetpp::cRuntimeError("intervalDuration is <= 0");

        // Schedule next light change event:
        scheduleAt(omnetpp::simTime().dbl() + intervalDuration, intervalChangeEVT);
    }
    else
        super::handleMessage(msg);
}


void TrafficLight_LQF_MWM_Aging::initialize_withTraCI()
{
    // call parent
    super::initialize_withTraCI();

    if(TLControlMode != TL_LQF_MWM_Aging)
        return;

    LOG_INFO << "\nMulti-class LQF-MWM-Aging traffic signal control ... \n" << std::flush;

    auto TLList = TraCI->TLGetIDList();

    // for each traffic light
    for (auto &TLid : TLList)
    {
        // get all incoming lanes
        auto lan = TraCI->TLGetControlledLanes(TLid);

        // remove duplicate entries
        sort( lan.begin(), lan.end() );
        lan.erase( unique( lan.begin(), lan.end() ), lan.end() );

        incomingLanes_perTL[TLid] = lan;

        // get all links controlled by this TL
        auto result = TraCI->TLGetControlledLinks(TLid);

        // for each link in this TLid
        for(auto &it2 : result)
        {
            int linkNumber = it2.first;
            std::vector<std::string> link = it2.second;
            std::string incommingLane = link[0];

            link2Lane.insert( std::make_pair(std::make_pair(TLid,linkNumber), incommingLane) );
        }
    }

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

    LOG_DEBUG << boost::format("\nSimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
    % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
}


void TrafficLight_LQF_MWM_Aging::executeEachTimeStep()
{
    // call parent
    super::executeEachTimeStep();

    if(TLControlMode != TL_LQF_MWM_Aging)
        return;
}


void TrafficLight_LQF_MWM_Aging::chooseNextInterval(std::string TLid)
{
    if (currentInterval == "yellow")
    {
        currentInterval = "red";

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
        // update TL status for this phase
        if(nextGreenInterval == firstGreen[TLid])
            updateTLstate(TLid, "phaseEnd", nextGreenInterval, true);  //todo: notion of cycle?
        else
            updateTLstate(TLid, "phaseEnd", nextGreenInterval);

        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState(TLid, nextGreenInterval);
        intervalDuration = minGreenTime;
    }
    else
        chooseNextGreenInterval(TLid);

    LOG_DEBUG << boost::format("\nSimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
    % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
}


void TrafficLight_LQF_MWM_Aging::chooseNextGreenInterval(std::string TLid)
{
    LOG_DEBUG << "\n------------------------------------------------------------------------------- \n";
    LOG_DEBUG << ">>> New phase calculation \n" << std::flush;

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

        LOG_DEBUG << "\n\n    Total delay of vehicles on each lane: \n";
        for(auto &lane : it->second)
        {
            auto vehs = laneGetQueue(lane).vehs;

            double totalDelay = 0;
            for(auto &veh : vehs)
            {
                delayEntry_t *vehDelay = vehicleGetDelay(veh.id,TLid);
                if(vehDelay)
                    totalDelay += vehDelay->totalDelay;
            }

            if(totalDelay != 0)
                LOG_DEBUG << boost::format("        lane %1%: %2% \n") % lane % totalDelay;
        }

        LOG_FLUSH;
    }

    priorityQ_weight sortedMovements;
    priorityQ_delay maxDelayPerPhase;

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
                // ignore this link if right turn
                if(!isRightTurn(linkNumber))
                {
                    // get the corresponding lane for this link
                    auto itt = link2Lane.find(std::make_pair(TLid,linkNumber));
                    if(itt == link2Lane.end())
                        throw omnetpp::cRuntimeError("linkNumber %d is not found in TL %s", linkNumber, TLid.c_str());
                    std::string lane = itt->second;

                    auto queuedVehs = laneGetQueue(lane).vehs;

                    for(auto &veh : queuedVehs)
                    {
                        // total weight of entities on this lane
                        auto loc = classWeight.find(veh.type);
                        if(loc == classWeight.end())
                            throw omnetpp::cRuntimeError("entity %s with type %s does not have a weight in classWeight map!", veh.id.c_str(), veh.type.c_str());

                        totalWeight += loc->second;

                        // max delay in this movement
                        delayEntry_t *vehDelay = vehicleGetDelay(veh.id,TLid);
                        if(vehDelay)
                            maxDelay = std::max(maxDelay, vehDelay->waitingDelay);  // todo: should we consider waiting time only?

                        vehCount++;
                    }
                }

                // number of movements in this phase (including right turns)
                oneCount++;
            }

            maxVehCount = std::max(maxVehCount, vehCount);
        }

        // add this batch of movements to sortedMovements
        sortedEntry_weight_t entry = {totalWeight, oneCount, maxVehCount, phase};
        sortedMovements.push(entry);

        // add this batch of movements to maxDelayPerPhase
        sortedEntry_delay_t entry2 = {totalWeight, oneCount, maxVehCount, maxDelay, phase};
        maxDelayPerPhase.push(entry2);
    }

    // get the movement batch with the highest weight + delay + oneCount
    sortedEntry_weight_t bestChoice = sortedMovements.top();

    // bestChoice is the phase with the highest total weight, but not necessarily the longest delay.
    // We need to check the max delay in each phase too!
    sortedEntry_weight_t finalChoice = bestChoice;
    double maxDelay = 0;
    while(!maxDelayPerPhase.empty())
    {
        maxDelay = maxDelayPerPhase.top().maxDelay;

        // do we exceed the 'max delay = 20s' in a phase?
        if(maxDelay + yellowTime + redTime > 20)
        {
            sortedEntry_delay_t delayChoice = maxDelayPerPhase.top();

            finalChoice.totalWeight = delayChoice.totalWeight;
            finalChoice.oneCount = delayChoice.oneCount;
            finalChoice.maxVehCount = delayChoice.maxVehCount;
            finalChoice.phase = delayChoice.phase;

            break;
        }

        maxDelayPerPhase.pop();
    }

    // allocate enough green time to move all vehicles
    int maxVehCount = finalChoice.maxVehCount;
    double greenTime = (double)maxVehCount * (minGreenTime / 5.);
    nextGreenTime = std::min(std::max(greenTime, minGreenTime), maxGreenTime);  // bound green time

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
        TraCI->TLSetState(TLid, nextInterval);

        intervalDuration =  yellowTime;

        // update TL status for this phase
        updateTLstate(TLid, "yellow");

        if(LOG_ACTIVE(DEBUG_LOG_VAL))
        {
            LOG_DEBUG << boost::format("\n    The following phase has the highest totalWeight out of %1% phases: \n") % phases.size();

            LOG_DEBUG << boost::format("        phase= %1%, maxVehCount= %2%, totalWeight= %3%, oneCount= %4%, green= %5% \n")
            % bestChoice.phase % bestChoice.maxVehCount % bestChoice.totalWeight % bestChoice.oneCount % nextGreenTime;

            if(bestChoice.phase != finalChoice.phase)
            {
                LOG_DEBUG << boost::format("\n    But it will not be scheduled! ");
                LOG_DEBUG << boost::format("\n    Because max delay in phase %1% (= %2%) will exceed %3%s \n") % finalChoice.phase % maxDelay % 20;
            }

            LOG_FLUSH;
        }
    }
    else
    {
        intervalDuration = nextGreenTime;
        LOG_DEBUG << "\n    Continue the last green interval. \n" << std::flush;
    }

    LOG_DEBUG << "------------------------------------------------------------------------------- \n" << std::flush;
}

}

/****************************************************************************/
/// @file    FMSC.cc
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

#include <queue>

#include <trafficLight/TSC/08_FMSC.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLight_FMSC);

class sortedEntry_FMSC
{
public:
    int oneCount;
    int maxVehCount;
    double totalDelay;
    double totalWeight;
    std::string phase;

    sortedEntry_FMSC(double d1, double d2, int i1, int i2, std::string p)
    {
        this->totalWeight = d1;
        this->totalDelay = d2;
        this->oneCount = i1;
        this->maxVehCount = i2;
        this->phase = p;
    }
};


class sortCompare_FMSC
{
public:
    bool operator()(sortedEntry_FMSC p1, sortedEntry_FMSC p2)
    {
        if( p1.totalWeight < p2.totalWeight )
            return true;
        else if( p1.totalWeight == p2.totalWeight && p1.totalDelay < p2.totalDelay)
            return true;
        else if( p1.totalWeight == p2.totalWeight && p1.totalDelay == p2.totalDelay && p1.oneCount < p2.oneCount)
            return true;
        else
            return false;
    }
};


// using a 'functor' rather than a 'function'
// Reason: to be able to pass an additional argument (bestMovement) to predicate
struct served_FMSC
{
public:
    served_FMSC(std::string str)
{
        this->thisPhase = str;
}

    bool operator () (const sortedEntry_FMSC v)
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


TrafficLight_FMSC::~TrafficLight_FMSC()
{

}


void TrafficLight_FMSC::initialize(int stage)
{
    super::initialize(stage);

    if(TLControlMode != TL_FMSC)
        return;

    if(stage == 0)
    {
        nextGreenIsNewCycle = false;
        intervalChangeEVT = new omnetpp::cMessage("intervalChangeEVT", 1);
    }
}


void TrafficLight_FMSC::finish()
{
    super::finish();
}


void TrafficLight_FMSC::handleMessage(omnetpp::cMessage *msg)
{
    if (TLControlMode == TL_FMSC && msg == intervalChangeEVT)
    {
        if(greenInterval.empty())
            calculatePhases("C");

        chooseNextInterval();

        if(intervalDuration <= 0)
            throw omnetpp::cRuntimeError("intervalDuration is <= 0");

        // Schedule next light change event:
        scheduleAt(omnetpp::simTime().dbl() + intervalDuration, intervalChangeEVT);
    }
    else
        super::handleMessage(msg);
}


void TrafficLight_FMSC::initialize_withTraCI()
{
    // call parent
    super::initialize_withTraCI();

    if(TLControlMode != TL_FMSC)
        return;

    LOG_INFO << "\nMulti-class LQF-MWM-Cycle traffic signal control ... \n" << std::flush;

    auto TLList = TraCI->TLGetIDList();

    // for each traffic light
    for (auto &TLid : TLList)
    {
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

    // find the RSU module that controls this TL
    RSUptr = findRSU("C");

    // make sure RSUptr is pointing to our corresponding RSU
    ASSERT(RSUptr);

    // turn on active detection on this RSU
    RSUptr->par("record_vehApproach_stat") = true;

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

        firstGreen[TL] = currentInterval;

        // initialize TL status
        updateTLstate(TL, "init", currentInterval);
    }

    LOG_DEBUG << boost::format("\n    SimTime: %1% | Planned interval: %2% | Start time: %1% | End time: %3% \n")
    % omnetpp::simTime().dbl() % currentInterval % (omnetpp::simTime().dbl() + intervalDuration) << std::flush;
}


void TrafficLight_FMSC::executeEachTimeStep()
{
    // call parent
    super::executeEachTimeStep();

    if(TLControlMode != TL_FMSC)
        return;
}


void TrafficLight_FMSC::chooseNextInterval()
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


void TrafficLight_FMSC::chooseNextGreenInterval()
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

    intervalDuration =  yellowTime;

    // update TL status for this phase
    updateTLstate("C", "yellow");
}


void TrafficLight_FMSC::calculatePhases(std::string TLid)
{
    LOG_DEBUG << "\n>>> New cycle calculation ... \n" << std::flush;

    // get all incoming lanes for this TL only
    std::map<std::string /*lane*/, laneInfoEntry> laneInfo = RSUptr->laneInfo;

    if(laneInfo.empty())
        throw omnetpp::cRuntimeError("LaneInfo is empty! Is active detection on in %s ?", RSUptr->getFullName());

    // batch of all non-conflicting movements, sorted by total weight + oneCount per batch
    std::priority_queue< sortedEntry_FMSC /*type of each element*/, std::vector<sortedEntry_FMSC> /*container*/, sortCompare_FMSC > sortedMovements;
    // clear the priority queue
    sortedMovements = std::priority_queue < sortedEntry_FMSC, std::vector<sortedEntry_FMSC>, sortCompare_FMSC >();

    for(std::string phase : phases)
    {
        double totalWeight = 0;  // total weight of all waiting entities in all movements in this phase
        double totalDelay = 0;  // total delay of all waiting entities in all movements in this phase
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
                    auto itt = link2Lane.find(std::make_pair(TLid,linkNumber));
                    if(itt == link2Lane.end())
                        throw omnetpp::cRuntimeError("linkNumber %s is not found in TL %s", linkNumber, TLid.c_str());
                    std::string lane = itt->second;

                    // find this lane in laneInfo
                    auto res = laneInfo.find(lane);
                    if(res == laneInfo.end())
                        throw omnetpp::cRuntimeError("Can not find lane %s in laneInfo!", lane.c_str());

                    // get all queued vehicles on this lane
                    auto vehicles = (*res).second.allVehicles;

                    // for each vehicle
                    for(auto& entry : vehicles)
                    {
                        double stoppingDelayThreshold = 0;
                        if(entry.second.vehType == "bicycle")
                            stoppingDelayThreshold = par("bikeStoppingDelayThreshold").doubleValue();
                        else
                            stoppingDelayThreshold = par("vehStoppingDelayThreshold").doubleValue();

                        // we only care about waiting vehicles on this lane
                        if(entry.second.currentSpeed <= stoppingDelayThreshold)
                        {
                            vehCount++;

                            std::string vID = entry.first;
                            std::string vType = entry.second.vehType;

                            // total weight of entities on this lane
                            auto loc = classWeight.find(vType);
                            if(loc == classWeight.end())
                                throw omnetpp::cRuntimeError("entity %s with type %s does not have a weight in classWeight map!", vID.c_str(), vType.c_str());
                            totalWeight += loc->second;

                            // total delay in this lane
                            delayEntry_t *vehDelay = vehicleGetDelay(vID,TLid);
                            if(vehDelay)
                                totalDelay += vehDelay->totalDelay;
                        }
                    }
                }

                // number of movements in this phase (including right turns)
                oneCount++;
            }

            maxVehCount = std::max(maxVehCount, vehCount);
        }

        // add this batch of movements to priority_queue
        sortedEntry_FMSC *entry = new sortedEntry_FMSC(totalWeight, totalDelay, oneCount, maxVehCount, phase);
        sortedMovements.push(*entry);
    }

    // copy sortedMovements to a vector for iteration:
    std::vector<sortedEntry_FMSC> batchMovementVector;
    while(!sortedMovements.empty())
    {
        batchMovementVector.push_back(sortedMovements.top());
        sortedMovements.pop();
    }

    // Select only the necessary phases for the new cycle:
    while(!batchMovementVector.empty())
    {
        // Always select the first movement because it will be the best(?):
        sortedEntry_FMSC entry = batchMovementVector.front();
        std::string nextInterval = entry.phase;

        greenInterval_FMSC *entry2 = new greenInterval_FMSC(entry.maxVehCount, entry.totalWeight, entry.oneCount, 0.0, entry.phase);
        greenInterval.push_back(*entry2);

        // Now delete these movements because they should never occur again:
        batchMovementVector.erase( std::remove_if(batchMovementVector.begin(), batchMovementVector.end(), served_FMSC(nextInterval)), batchMovementVector.end() );
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
            [](const greenInterval_FMSC v)
            {
        if (v.greenTime == 0.0)
            return true;
        else
            return false;
            }
    );
    greenInterval.erase( rme, greenInterval.end() );
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
            LOG_DEBUG << "        movement: " << i.greenString
            << ", maxVehCount= " << i.maxVehCount
            << ", totalWeight= " << i.totalWeight
            << ", oneCount= " << i.oneCount
            << ", green= " << i.greenTime << "s \n";

        LOG_FLUSH;
    }
}

}

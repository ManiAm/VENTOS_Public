/****************************************************************************/
/// @file    TrafficLightRouter.cc
/// @author  Dylan Smith <dilsmith@ucdavis.edu>
/// @author  second author here
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

#include "08_TL_Router.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightRouter);

Phase::Phase(double duration, std::string state): duration(duration), state(state){}

void Phase::print() // Print a phase
{
    if(ev.isGUI()) std::cout << "duration: "<< std::setw(4) << std::left << duration << " phase: " << std::setw(12) << std::left << state << std::endl;
}

std::string setState(std::string state, int index, char value)    //Writes given char at specified index on the state,
{                                                       //extending with '_' if necessary, and returns it
    while((unsigned int)index >= state.length())                      //extend to necessary length
    {
        state = state + '_';
    }

    state[index] = value;
    return state;
}

class VState
{
public:
    static constexpr double MAX_ACCEL = 3.0;
    static constexpr double MAX_VELOCITY = 30.0;

    double position;
    double eta;
    std::string id;

    /*
    VState(double position, double velocity, double acceleration, string id) : position(position), id(id)
    {
        eta = (sqrt(2 * acceleration * position + velocity * velocity) -velocity)/acceleration;
        eta = position;
    }
    */

    VState(std::string vehicle, TraCI_Extend *TraCI, Router* router, std::string currentEdge, double nextEdgeLength = 0): id(vehicle)
    {
        position = router->net->edges[currentEdge]->length - TraCI->vehicleGetLanePosition(vehicle) + nextEdgeLength;//position is distance from the TL along roads
        double velocity = TraCI->vehicleGetSpeed(vehicle);

        double accelTime = (MAX_VELOCITY - velocity) / MAX_ACCEL;
        double averageVelocityDuringAccel = (MAX_VELOCITY + velocity) / 2;
        double accelDistance =accelTime * averageVelocityDuringAccel;   //How far we travel while accelerating to the max speed

        if(accelDistance < position)    //If we still have distance to cover
        {
            eta = accelTime + ((position - accelDistance) / MAX_VELOCITY);
        }
        else    //Need to calculate assuming we never reach max accel
        {
            eta = (sqrt(2 * MAX_ACCEL * position + velocity * velocity) -velocity)/MAX_ACCEL;
        }
    }

    /*
    double position;
    double velocity;
    double acceleration;
    string id;

    VState(double position, double velocity, double acceleration, string id) :
        position(position), velocity(velocity), acceleration(acceleration), id(id)
    {}

        void print()
    {
        cout << id << ": " << endl
             << "    pos: " << position << endl
             << "    vel: " << velocity << endl
             << "    acc: " << acceleration << endl;
    }
    */

    bool operator<(const VState& rhs) const
    {
        return this->position < rhs.position;
    }
};

//Contains a vector of vehicles (provides basic vector operations) and some helpful operations (getFlow)
class VStateContainer
{
    std::vector<VState> v;
    int index;

public:
    void push_back(VState vs)
    {
        v.push_back(vs);
    }

    std::vector<VState>::iterator begin()
    {
        return v.begin();
    }

    std::vector<VState>::iterator end()
    {
        return v.end();
    }

    int size()
    {
        return v.size();
    }

    VStateContainer()
    {
        index = 0;
    }

    //TODO: make this a binary search, since the list is sorted
    //Determines the average vehicle passing rate (# vehicles / time) for this vehicle set, given a time
    double flowRate(int time)
    {
        index = 0;
        if(v[v.size()-1].eta < time)
            return (double)v.size() / time;
        if(v[0].eta > time)
            return 0.0 / time;
        else
        {
            int i = 0;
            while(v[i].eta < time)
                i++;

            return (double)i/time;
        }
        /*
        while(index < v.size() and v[index].eta < time)
            index++;
        while(index >= 0 and v[index].eta > time)
            index--;

        if(index == v.size())
            index--;

        int numVehicles = index + 1;
        return (double)numVehicles / time;*/
    }
};


TrafficLightRouter::TrafficLightRouter()    //Crashes immediately upon execution if this isn't here o.O
{

}

void TrafficLightRouter::build(std::string id, std::string type, std::string programID, double offset, std::vector<Phase*>& phases, Net* net)
{
    this->id = id;
    this->type = type;
    this->programID = programID;
    this->offset = offset;
    this->phases = phases;
    this->net = net;

    done = false;
    currentPhase = 0;
    lastSwitchTime = 0;
    cycleDuration = 0;
    isTransitionPhase = false;
    nonTransitionalCycleDuration = 0;

    for(unsigned int i = 0; i < phases.size(); i++)
    {
        cycleDuration += phases[i]->duration;
        if(i % 2 == 0)
            nonTransitionalCycleDuration += phases[i]->duration;
    }
}

void TrafficLightRouter::initialize(int stage)
{
    TrafficLightVANET::initialize(stage);

    if(TLControlMode != TL_Router)
        return;

    if(id == "")
        return;

    if(stage == 0)
    {
        TLLogicMode = static_cast<TrafficLightLogicMode>(par("TLLogicMode").longValue());
        HighDensityRecalculateFrequency = par("HighDensityRecalculateFrequency").doubleValue();
        LowDensityExtendTime = par("LowDensityExtendTime").doubleValue();
        MaxPhaseDuration = par("MaxPhaseDuration").doubleValue();
        MinPhaseDuration = par("MinPhaseDuration").doubleValue();

        currentPhase = 0;
        switch(TLLogicMode)
        {
        case FIXED:
            break;

        case HIGHDENSITY:
            TLEvent = new cMessage("tl evt");   //Create a new internal message
            scheduleAt(simTime() + HighDensityRecalculateFrequency, TLEvent); //Schedule them to start sending
            TLSwitchEvent = new cMessage("tl switch evt");
            scheduleAt(phases[0]->duration, TLSwitchEvent);
            break;

        case LOWDENSITY:
            TLSwitchEvent = new cMessage("tl switch evt");
            scheduleAt(phases[0]->duration, TLSwitchEvent);
            break;

        case COOPERATIVE:
            TLSwitchEvent = new cMessage("tl switch evt");
            scheduleAt(1, TLSwitchEvent);
        }

        cModule *rmodule = simulation.getSystemModule()->getSubmodule("router");
        router = static_cast< Router* >(rmodule);
    }
}


//Switch tl to the specified phase
void TrafficLightRouter::switchToPhase(int phase, double greenDuration, int yellowDuration)
{
    int yellowPhase = (currentPhase + 1) % phases.size();   //Identify the yellowphase that follows the current phase
    phase = phase % phases.size();
    currentPhase = phase;

    if(greenDuration < 0)   //If greenDuration is less than zero, assume default duration
        greenDuration = phases[phase]->duration;
    phaseDurationAfterTransition = greenDuration;
    isTransitionPhase = true;   //Mark that the next phase is a short yellow (transitional) phase

    lastSwitchTime = simTime().dbl(); //Mark the last switch time

    TraCI->TLSetPhaseIndex(id, yellowPhase);           //Manually switch to the yellow phase in SUMO
    TLSwitchEvent = new cMessage("tl switch evt");
    scheduleAt(simTime().dbl() + yellowDuration, TLSwitchEvent);
    TraCI->TLSetPhaseDuration(id, 10000000);   //Make sure SUMO doesn't handle switches, by always setting phases to expire after a very long time
}

void TrafficLightRouter::handleMessage(cMessage* msg)  //Internal messages to self
{
    TrafficLightVANET::handleMessage(msg);

    if(TLControlMode != TL_Router)
        return;

    if(!done)
    {
        if(msg->isName("tl evt"))   //Out-of-sync TL Algorithms take place here
        {
            ASynchronousMessage();
        }
        else if(msg->isName("tl switch evt"))   //Operations in sync with normal phase switching happens here
        {
            SynchronousMessage();
        }
    }
    delete msg;
}

//Messages sent whenever a phase expires
void TrafficLightRouter::SynchronousMessage()
{
    if(isTransitionPhase)   //If the previous phase was a transition
    {
        isTransitionPhase = false;
        lastSwitchTime = simTime().dbl();
        TraCI->TLSetPhaseIndex(id, currentPhase);           //currentPhase was set to the phase after the transition, so switch to it
        TLSwitchEvent = new cMessage("tl switch evt");
        scheduleAt(simTime().dbl() + phaseDurationAfterTransition, TLSwitchEvent);
        TraCI->TLSetPhaseDuration(id, 10000000);   //Make sure SUMO doesn't handle switches, by always setting phases to expire after a very long time
    }
    else
    {
        switch(TLLogicMode)
        {
            case FIXED:
            case HIGHDENSITY:
                switchToPhase(currentPhase + 2);
                break;
            case LOWDENSITY:
                LowDensityRecalculate();
                break;
            case COOPERATIVE:
                FlowRateRecalculate();
                break;
            default:
                break;
        }
    }
}

//Messages sent at other times
void TrafficLightRouter::ASynchronousMessage()
{
    if(TLLogicMode == HIGHDENSITY)
    {
        HighDensityRecalculate();   //Call the recalculate function, and schedule the next execution
        TLEvent = new cMessage("tl evt");
        scheduleAt(simTime() + HighDensityRecalculateFrequency, TLEvent);
    }
}

void TrafficLightRouter::HighDensityRecalculate()
{
    std::vector<Edge*>& edges = net->nodes[id]->inEdges;
    double phaseVehicleCounts[phases.size()];   //Will be the # of incoming vehicles that can go during that phase
    for(unsigned int i = 0; i < phases.size(); i++)  //Initialize to 0
        phaseVehicleCounts[i] = 0;
    int total = 0;

    for(std::vector<Edge*>::iterator edge = edges.begin(); edge != edges.end(); edge++)  //For each edge
    {
        std::vector<Lane*>* lanes = &(*edge)->lanes;
        for(std::vector<Lane*>::iterator lane = lanes->begin(); lane != lanes->end(); lane++)    //For each lane
        {
            std::list<std::string> vehicleIDs = TraCI->laneGetLastStepVehicleIDs((*lane)->id); //Get all vehicles on that lane
            int vehCount = vehicleIDs.size();   //And the number of vehicles on that lane
            for(std::vector<int>::iterator it = (*lane)->greenPhases.begin(); it != (*lane)->greenPhases.end(); it++)    //Each element of greenPhases is a phase that lets that lane move
            {
                phaseVehicleCounts[*it] += vehCount;    //Add the number of vehicles on that lane to that phase
                total += vehCount; //And add to the totals
                    //If we  want each vehicle to contribute exactly 1 weight, add vehCount/greenPhases.size() instead.
            }
        }
    }
    if(total > 0)  //If there are vehicles on the lane
    {
        if(ev.isGUI()) std::cout << "For TL " << id << ": " << endl;
        for(unsigned int i = 0; i < phases.size(); i++)  //For each phase
        {
            if(i % 2 == 0)  //Ignore the odd (transitional) phases
            {
                double portion = (phaseVehicleCounts[i]/total) * 2 ;         //The portion of time allotted to that lane should be how many vehicles
                double duration = portion * nonTransitionalCycleDuration;   //can move during that phase divided by the total number of vehicles
                if(duration < 3)    //If the duration is too short, set it to a minimum
                    duration = 3;

                //if(ev.isGUI()) cout << "    Phase " << i << " set to " << duration << endl;
                phases[i]->duration = duration; //Update durations. These will take affect starting with the next phase
            }
        }
    }
}


void TrafficLightRouter::FlowRateRecalculate()
{
    std::vector<Edge*>* inEdges = &(router->net->nodes[id]->inEdges);    //inEdges is the edges leading to the TL
    std::vector<Edge*>* outEdges = &(router->net->nodes[id]->outEdges);  //outEdges is the edges exiting from the TL
    std::map<std::string, VStateContainer> movements;                         //All 16 possible in-out edge combinations

    for(Edge*& inEdge : *inEdges)
        for(Edge*& outEdge : *outEdges)
        {
            std::string key = inEdge->id + outEdge->id;
            movements[key];    //Initialize the 16 movement vectors in the map
        }

    for(Edge*& edge1 : *inEdges)
    {
        for(Lane*& lane1 : edge1->lanes) //For each lane on those edges
        {

            //TraCI->laneGetIDList(lane1->id)
            for(std::string& vehicle : TraCI->laneGetLastStepVehicleIDs(lane1->id))   //For each vehicle on those lanes
            {
                std::list<std::string> route = TraCI->vehicleGetRoute(vehicle);    //Get the vehicle's route
                while(route.front().compare(edge1->id) != 0) //Remove edges it's already traveled (TODO: this doesn't check for cycles!)
                    route.pop_front();

                if(route.size() > 1)    //If 1 entry, vehicle will vanish at the end of the edge, so we don't count it
                {
                    VState v(vehicle, TraCI, router, edge1->id);
                    std::string edge0 = *(++(route.begin()));     //Edge vehicle is turning towards (ourEdge)
                    movements[edge1->id + edge0].push_back(v);
                }//If vehicle will not vanish before passing through our TL
            }//For each vehicle on lane
        }//For each lane on edge1

        Node* outerTLNode = edge1->from;

        std::string state = TraCI->TLGetState(outerTLNode->id);
        for(Edge*& edge2 : outerTLNode->inEdges)
        {
            for(Lane*& lane2 : edge2->lanes) //For each lane on those edges
            {
                for(std::string& vehicle : TraCI->laneGetLastStepVehicleIDs(lane2->id))   //For each vehicle on those lanes
                {
                    std::list<std::string> route = TraCI->vehicleGetRoute(vehicle);    //Get the vehicle's route
                    while(route.front().compare(edge2->id) != 0) //Remove edges it's already traveled (TODO: this doesn't check for cycles!)
                        route.pop_front();

                    if(route.size() > 2)    //If < 3 entry, vehicle will vanish at the end of the edge, so we don't count it
                    {
                        std::string vehNextEdge = *(++(route.begin()));      //Edge vehicle is turning towards
                        if(vehNextEdge == edge1->id)                    //If vehicle is turning towards our TL
                        {
                            for(Connection*& c : router->net->connections[edge2->id + edge1->id])   //For each connection between these edges
                            {
                                if(state[c->linkIndex] == 'G' or state[c->linkIndex] == 'g')
                                {
                                    VState v(vehicle, TraCI, router, edge1->id, router->net->edges[edge1->id]->length);
                                    v.position += edge1->length;
                                    std::string edge0 = *(++(++(route.begin())));     //Edge after the vehicle moves through our TL
                                    movements[edge1->id + edge0].push_back(v);
                                    break;
                                }//If vehicle can turn from edge2 to edge1
                            }//For each connection between edge2 and edge1
                        }//If vehicle is turning from edge2 to edge1
                    }//If vehicle will not vanish before making it through our tl
                }//For each vehicle
            }//For each lane on edge2
        }//For each edge going towards the outer node
    }//For each lane going towards our TL

    for(auto& pair : movements)
    {
        std::string key = pair.first;
        VStateContainer* movement = &(pair.second);
        sort(movement->begin(), movement->end());   //Sort vehicle by position

        double prev = 0;
        for(VState& vehicle : *movement)    //Ensure each vehicle's ETA is at least as long as its predecessor's
        {
            if(vehicle.eta < prev)
                vehicle.eta = prev;
            prev = vehicle.eta;
        }
    }


    int maxFlowTime = -1;
    int maxFlowPhase = -1;
    double maxFlowSum = 0;
    double flowSum = 0;
    for(int t = MinPhaseDuration; t <= MaxPhaseDuration; t += 5)
    {
        //if(id == "15") cout << "t=" << t << endl;
        for(unsigned int phaseNum = 0; phaseNum < phases.size(); phaseNum += 2)
        {
            Phase* phase = phases[phaseNum];
            std::string state = phase->state;
            for(auto& pair : movements)
            {
                std::string key = pair.first;
                VStateContainer* movement = &(pair.second);
                if(movement->size() > 0)
                {
                    for(Connection*& c : router->net->connections[key]) //For each connection between the edges
                    {
                        if(state[c->linkIndex] == 'g' or state[c->linkIndex] == 'G')    //If there's a green light
                        {
                            flowSum += movement->flowRate(t);   //Movement is allowed, so add the flow to the total for the phase
                            break;
                        }
                    }
                }
                //if(id == "15") cout << "  phase=" << phaseNum << endl << "    flow=" << flowSum << endl;
            }

            if(flowSum > maxFlowSum)    //if this phase has the hisghest net flow, record it as such
            {
                maxFlowTime = t;
                maxFlowPhase = phaseNum;
                maxFlowSum = flowSum;
            }
            flowSum = 0;
        }//For each phase
    }//For each time

    if(maxFlowTime >= MinPhaseDuration) //if any vehicles will arrive before the light's max duration, maxFlowTime will have been set
    {
        switchToPhase(maxFlowPhase, maxFlowTime);   //So switch to that phase
        //cout << "Switching " << id << " to phase " << maxFlowPhase << " for " << maxFlowTime << " seconds" << endl;
    }
    else    //No vehicles are nearby. Continue TL operations as normal.
        switchToPhase(currentPhase + 1);
}


void TrafficLightRouter::LowDensityRecalculate()
{
    if(simTime().dbl() - lastSwitchTime < MaxPhaseDuration and LowDensityVehicleCheck())
    {
        //if(ev.isGUI()) cout << "Extending tl " << id << " phase " << currentPhase << endl;
        TLSwitchEvent = new cMessage("tl switch evt");
        scheduleAt(simTime().dbl() + LowDensityExtendTime, TLSwitchEvent);
        TraCI->TLSetPhaseDuration(id, 10000000);
    }
    else    //Otherwise, switch to the next phase immediately
    {
        switchToPhase(currentPhase + 2);
    }
}


bool TrafficLightRouter::LowDensityVehicleCheck()    //This function assumes it's always LowDensityExtendTime away from the current phase ending,
{                                             //and that the next is transitional.  Returns how much time was added/subtracted from the phase
    /*
     * Get a list of all vehicles on lanes with greens
     * Check if any vehicle is X seconds from finishing
     * If so, add X to the time and return X
     *
     * Remember, the extended time needs to be cleared as well
     * Add a totaloffset double to each variable, which can be added to any call working on tl logic
     * Reset to previous duration
     */

    std::vector<Edge*>& edges = net->nodes[id]->inEdges; //Get all edges going into the TL
    for(std::vector<Edge*>::iterator edge = edges.begin(); edge != edges.end(); edge++)  //For each edge
    {
        std::vector<Lane*>* lanes = &(*edge)->lanes; //Get all lanes on each edge
        for(std::vector<Lane*>::iterator lane = lanes->begin(); lane != lanes->end(); lane++)    //For each lane
        {
            if(find((*lane)->greenPhases.begin(), (*lane)->greenPhases.end(), currentPhase) != (*lane)->greenPhases.end()) //If this lane has a green
            {
                std::list<std::string> vehicleIDs = TraCI->laneGetLastStepVehicleIDs((*lane)->id); //If so, get all vehicles on this lane
                for(std::list<std::string>::iterator vehicle = vehicleIDs.begin(); vehicle != vehicleIDs.end(); vehicle++)    //For each vehicle
                {
                    if(TraCI->vehicleGetSpeed(*vehicle) > 0.01)  //If that vehicle is not stationary
                    {
                        double pos = TraCI->vehicleGetLanePosition(*vehicle);
                        double length = net->edges[(*edge)->id]->length;
                        double speed = net->edges[(*edge)->id]->speed;
                        double timeLeft = (length - pos) / speed;   //Calculate how long until it hits the intersection, based off speed and distance
                        if(timeLeft < LowDensityExtendTime) //If this is within LowDensityExtendTime
                        {
                            return true;    //We have found a vehicle that will benefit from extending
                        }
                    }
                }
            }
        }
    }
    return false;
}


void TrafficLightRouter::finish()
{
    TrafficLightVANET::finish();

    if(TLControlMode != TL_Router)
        return;

    done = true;
}


void TrafficLightRouter::print() // Print a node
{
    if(ev.isGUI()) std::cout<<"id: "<< std::setw(4) << std::left << id <<
                              "type: " << std::left << type <<
                              "  programID: "<< std::setw(4) << std::left << programID <<
                              "offset: "<< std::setw(4) << std::left << offset;

    for(std::vector<Phase*>::iterator it = phases.begin(); it != phases.end(); it++)
    {
        (*it)->print();
    }
}


int TrafficLightRouter::currentPhaseAtTime(double time, double* timeRemaining)
{
    int phase = currentPhase;
    int curTime = lastSwitchTime + phases[phase]->duration; //Start at the next switch
    while(time >= curTime)  //While the time requested is less than curTime. When this breaks, our phase is set to the current phase
    {
        phase = (phase + 1) % phases.size();  //Move to the next phase
        curTime += phases[phase]->duration; //And add that duration to curTime
    }
    if(timeRemaining != NULL)   //If timeRemaining was passed in
        *timeRemaining = curTime - time;

    return phase;
}

}


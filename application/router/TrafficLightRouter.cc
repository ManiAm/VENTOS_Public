
#include "TrafficLightRouter.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightRouter);

Phase::Phase(double duration, string state):
        duration(duration), state(state){}

void Phase::print() // Print a phase
{
    cout << "duration: "<< setw(4) << left << duration <<
         " phase: " << setw(12) << left << state << endl;
}

string setState(string state, int index, char value)    //Writes given char at specified index on the state,
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
    string id;

    /*
    VState(double position, double velocity, double acceleration, string id) : position(position), id(id)
    {
        eta = (sqrt(2 * acceleration * position + velocity * velocity) -velocity)/acceleration;
        eta = position;
    }
    */

    VState(string vehicle, TraCI_Extend *TraCI, Router* router, string currentEdge, double nextEdgeLength = 0): id(vehicle)
    {
        position = router->net->edges[currentEdge]->length - TraCI->commandGetVehicleLanePosition(vehicle) + nextEdgeLength;//position is distance from the TL along roads
        double velocity = TraCI->commandGetVehicleSpeed(vehicle);

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
    vector<VState> v;
    int index;

public:
    void push_back(VState vs)
    {
        v.push_back(vs);
    }

    vector<VState>::iterator begin()
    {
        return v.begin();
    }

    vector<VState>::iterator end()
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

void TrafficLightRouter::build(string id, string type, string programID, double offset, vector<Phase*>& phases, Net* net)
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
    if(id == "")
        return;

    if(stage == 0)
    {
        TLLogicMode = par("TLLogicMode").longValue();
        HighDensityRecalculateFrequency = par("HighDensityRecalculateFrequency").doubleValue();
        LowDensityExtendTime = par("LowDensityExtendTime").doubleValue();
        MaxPhaseDuration = par("MaxPhaseDuration").doubleValue();
        MinPhaseDuration = par("MinPhaseDuration").doubleValue();

        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        if(TLLogicMode == 1)
        {
            TLEvent = new cMessage("tl evt");   //Create a new internal message
            scheduleAt(simTime() + HighDensityRecalculateFrequency, TLEvent); //Schedule them to start sending
            TLSwitchEvent = new cMessage("tl switch evt");
            scheduleAt(phases[0]->duration, TLSwitchEvent);
        }
        else if(TLLogicMode == 2)
        {
            TLSwitchEvent = new cMessage("tl switch evt");
            scheduleAt(phases[0]->duration, TLSwitchEvent);
        }
        else if(TLLogicMode == 3)
        {
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

    TraCI->commandSetTLPhaseIndex(id, yellowPhase);           //Manually switch to the yellow phase in SUMO
    TLSwitchEvent = new cMessage("tl switch evt");
    scheduleAt(simTime().dbl() + yellowDuration, TLSwitchEvent);
    TraCI->commandSetTLPhaseDurationRemaining(id, 10000000);   //Make sure SUMO doesn't handle switches, by always setting phases to expire after a very long time
}

void TrafficLightRouter::handleMessage(cMessage* msg)  //Internal messages to self
{
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
        TraCI->commandSetTLPhaseIndex(id, currentPhase);           //currentPhase was set to the phase after the transition, so switch to it
        TLSwitchEvent = new cMessage("tl switch evt");
        scheduleAt(simTime().dbl() + phaseDurationAfterTransition, TLSwitchEvent);
        TraCI->commandSetTLPhaseDurationRemaining(id, 10000000);   //Make sure SUMO doesn't handle switches, by always setting phases to expire after a very long time
    }
    else
    {
        switch(TLLogicMode)
        {
            case 2:
                LowDensityRecalculate();
                break;
            case 3:
                FlowRateRecalculate();
                break;
        }
    }
}

//Messages sent at other times
void TrafficLightRouter::ASynchronousMessage()
{
    if(TLLogicMode == 1)
    {
        HighDensityRecalculate();   //Call the recalculate function, and schedule the next execution
        TLEvent = new cMessage("tl evt");
        scheduleAt(simTime() + HighDensityRecalculateFrequency, TLEvent);
    }
}

void TrafficLightRouter::HighDensityRecalculate()
{
    vector<Edge*>& edges = net->nodes[id]->inEdges;
    double phaseVehicleCounts[phases.size()];   //Will be the # of incoming vehicles that can go during that phase
    for(unsigned int i = 0; i < phases.size(); i++)  //Initialize to 0
        phaseVehicleCounts[i] = 0;
    int total = 0;

    for(vector<Edge*>::iterator edge = edges.begin(); edge != edges.end(); edge++)  //For each edge
    {
        vector<Lane*>* lanes = &(*edge)->lanes;
        for(vector<Lane*>::iterator lane = lanes->begin(); lane != lanes->end(); lane++)    //For each lane
        {
            list<string> vehicleIDs = TraCI->commandGetLaneVehicleList((*lane)->id); //Get all vehicles on that lane
            int vehCount = vehicleIDs.size();   //And the number of vehicles on that lane
            for(vector<int>::iterator it = (*lane)->greenPhases.begin(); it != (*lane)->greenPhases.end(); it++)    //Each element of greenPhases is a phase that lets that lane move
            {
                phaseVehicleCounts[*it] += vehCount;    //Add the number of vehicles on that lane to that phase
                total += vehCount; //And add to the total
                    //If we  want each vehicle to contribute exactly 1 weight, add vehCount/greenPhases.size() instead.
            }
        }
    }
    if(total > 0)  //If there are vehicles on the lane
    {
        for(unsigned int i = 0; i < phases.size(); i++)  //For each phase
        {
            if(i % 2 == 0)  //Ignore the odd (transitional) phases
            {
                double portion = (phaseVehicleCounts[i]/total) * 2;         //The portion of time allotted to that lane should be how many vehicles
                double duration = portion * nonTransitionalCycleDuration;   //can move during that phase divided by the total number of vehicles
                if(duration < 3)    //If the duration is too short, set it to a minimum
                    duration = 3;

                //cout << "    Phase " << i << " set to " << duration << endl;
                phases[i]->duration = duration; //Update durations. These will take affect starting with the next phase
            }
        }
    }
}


void TrafficLightRouter::FlowRateRecalculate()
{
    vector<Edge*>* inEdges = &(router->net->nodes[id]->inEdges);    //inEdges is the edges leading to the TL
    vector<Edge*>* outEdges = &(router->net->nodes[id]->outEdges);  //outEdges is the edges exiting from the TL
    map<string, VStateContainer> movements;                         //All 16 possible in-out edge combinations

    for(Edge*& inEdge : *inEdges)
        for(Edge*& outEdge : *outEdges)
        {
            string key = inEdge->id + outEdge->id;
            movements[key];    //Initialize the 16 movement vectors in the map
        }

    for(Edge*& edge1 : *inEdges)
    {
        for(Lane*& lane1 : edge1->lanes) //For each lane on those edges
        {
            for(string& vehicle : TraCI->commandGetLaneVehicleList(lane1->id))   //For each vehicle on those lanes
            {
                list<string> route = TraCI->commandGetVehicleEdgeList(vehicle);    //Get the vehicle's route
                while(route.front().compare(edge1->id) != 0) //Remove edges it's already traveled (TODO: this doesn't check for cycles!)
                    route.pop_front();

                if(route.size() > 1)    //If 1 entry, vehicle will vanish at the end of the edge, so we don't count it
                {
                    VState v(vehicle, TraCI, router, edge1->id);
                    string edge0 = *(++(route.begin()));     //Edge vehicle is turning towards (ourEdge)
                    movements[edge1->id + edge0].push_back(v);
                }//If vehicle will not vanish before passing through our TL
            }//For each vehicle on lane
        }//For each lane on edge1

        Node* outerTLNode = edge1->from;

        string state = TraCI->commandGetTLState(outerTLNode->id);
        for(Edge*& edge2 : outerTLNode->inEdges)
        {
            for(Lane*& lane2 : edge2->lanes) //For each lane on those edges
            {
                for(string& vehicle : TraCI->commandGetLaneVehicleList(lane2->id))   //For each vehicle on those lanes
                {
                    list<string> route = TraCI->commandGetVehicleEdgeList(vehicle);    //Get the vehicle's route
                    while(route.front().compare(edge2->id) != 0) //Remove edges it's already traveled (TODO: this doesn't check for cycles!)
                        route.pop_front();

                    if(route.size() > 2)    //If < 3 entry, vehicle will vanish at the end of the edge, so we don't count it
                    {
                        string vehNextEdge = *(++(route.begin()));      //Edge vehicle is turning towards
                        if(vehNextEdge == edge1->id)                    //If vehicle is turning towards our TL
                        {
                            for(Connection*& c : router->net->connections[edge2->id + edge1->id])   //For each connection between these edges
                            {
                                if(state[c->linkIndex] == 'G' or state[c->linkIndex] == 'g')
                                {
                                    VState v(vehicle, TraCI, router, edge1->id, router->net->edges[edge1->id]->length);
                                    v.position += edge1->length;
                                    string edge0 = *(++(++(route.begin())));     //Edge after the vehicle moves through our TL
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
        string key = pair.first;
        VStateContainer* movement = &(pair.second);
        sort(movement->begin(), movement->end());   //Sort vehicle by position

        double prev = 0;
        for(VState& vehicle : *movement)    //Ensure each vehicle's ETA is at least as long as its predecessor's
        {
            if(vehicle.eta < prev)
                vehicle.eta = prev;
            prev = vehicle.eta;
        }

        /*
        if(movement->size() > 0)
        {
            cout << "For tl " << id << " at t=" << simTime().dbl() << ", movement " << key << endl;
            for(VState& vehicle : *movement)
                cout << "    " << vehicle.id << " " << vehicle.position << " " << vehicle.eta << " " << movement->flowRate(20) << endl;
        }*/
    }

    /*
     *  for each phase
     *      for each movement
     *          if movement is possible in that phase (there exists a connection for it which has a linkindex that is green in the tl state)
     *              add to flowSum for that phase and time
     */

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
            string state = phase->state;
            for(auto& pair : movements)
            {
                string key = pair.first;
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
        cout << "Extending tl " << id << " phase " << currentPhase << endl;
        TLSwitchEvent = new cMessage("tl switch evt");
        scheduleAt(simTime().dbl() + LowDensityExtendTime, TLSwitchEvent);
        TraCI->commandSetTLPhaseDurationRemaining(id, 10000000);
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

    /*
    int phase = net->TLs[id]->currentPhase;
    string phaseState = net->TLs[id]->phases[phase]->state;
    for(Edge*& inEdge : net->nodes[id]->inEdges)
    {
        for(Edge*& outEdge : net->nodes[id]->outEdges)
        {
            for(Connection*& c : net->connections[inEdge->id + outEdge->id])
            {
                if(state[c->linkIndex] == 'g' || state[c->linkIndex] == 'G')
                {

                }
            }
        }
    }*/

    vector<Edge*>& edges = net->nodes[id]->inEdges; //Get all edges going into the TL
    for(vector<Edge*>::iterator edge = edges.begin(); edge != edges.end(); edge++)  //For each edge
    {
        vector<Lane*>* lanes = &(*edge)->lanes; //Get all lanes on each edge
        for(vector<Lane*>::iterator lane = lanes->begin(); lane != lanes->end(); lane++)    //For each lane
        {
            if(find((*lane)->greenPhases.begin(), (*lane)->greenPhases.end(), currentPhase) != (*lane)->greenPhases.end()) //If this lane has a green
            {
                list<string> vehicleIDs = TraCI->commandGetLaneVehicleList((*lane)->id); //If so, get all vehicles on this lane
                for(list<string>::iterator vehicle = vehicleIDs.begin(); vehicle != vehicleIDs.end(); vehicle++)    //For each vehicle
                {
                    if(TraCI->commandGetVehicleSpeed(*vehicle) > 0.01)  //If that vehicle is not stationary
                    {
                        double pos = TraCI->commandGetVehicleLanePosition(*vehicle);
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
    done = true;
    /*
    if(UseTLLogic)
    {
        if(UseHighDensityLogic)
        {
            if (TLEvent->isScheduled())
            {
                //cancelAndDelete(TLEvent);
            }
            else
            {
                //delete TLEvent;
            }
        }
        if (TLSwitchEvent->isScheduled())
        {
            cancelAndDelete(TLSwitchEvent);
        }
        else
        {
            delete TLSwitchEvent;
        }
    }*/
}


void TrafficLightRouter::print() // Print a node
{
    cout<<"id: "<< setw(4) << left << id <<
          "type: " << left << type <<
          "  programID: "<< setw(4) << left << programID <<
          "offset: "<< setw(4) << left << offset;
    for(vector<Phase*>::iterator it = phases.begin(); it != phases.end(); it++)
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


#include "TrafficLight.h"

namespace VENTOS {

Define_Module(VENTOS::TrafficLight);

Phase::Phase(double duration, string state):
        duration(duration), state(state){}

void Phase::print() // Print a phase
{
    cout << "duration: "<< setw(4) << left << duration <<
         " phase: " << setw(12) << left << state << endl;
}

TrafficLight::TrafficLight()    //Crashes immediately upon execution if this isn't here o.O
{

}

void TrafficLight::build(string id, string type, string programID, double offset, vector<Phase*>& phases, Net* net)
{
    this->id = id;
    this->type = type;
    this->programID = programID;
    this->offset = offset;
    this->phases = phases;
    this->net = net;

    currentPhase = 0;
    lastSwitchTime = 0;
    cycleDuration = 0;
    nonTransitionalCycleDuration = 0;

    for(int i = 0; i < phases.size(); i++)
    {
        cycleDuration += phases[i]->duration;
        if(i % 2 == 0)
            nonTransitionalCycleDuration += phases[i]->duration;
    }
}

void TrafficLight::initialize(int stage)
{
    if(id == "")
        return;

    if(stage == 0)
    {
        UseTLLogic = par("UseTLLogic").boolValue();
        UseHighDensityLogic = par("UseHighDensityLogic").boolValue();
        HighDensityRecalculateFrequency = par("HighDensityRecalculateFrequency").doubleValue();
        LowDensityExtendTime = par("LowDensityExtendTime").doubleValue();

        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        if (UseTLLogic)
        {
            if (UseHighDensityLogic)
            {
                TLEvent = new cMessage("tl evt");   //Create a new internal message
                scheduleAt(simTime() + HighDensityRecalculateFrequency, TLEvent); //Schedule them to start sending
            }

            TLSwitchEvent = new cMessage("tl switch evt");
            scheduleAt(phases[0]->duration, TLSwitchEvent);
        }
    }
}

void TrafficLight::handleMessage(cMessage* msg)  //Internal messages to self
{
    if(msg->isName("tl evt"))   //Out-of-sync TL Algorithms take place here
    {
        if(UseHighDensityLogic)
        {
            HighDensityRecalculate();   //Call the recalculate function, and schedule the next execution
            TLEvent = new cMessage("tl evt");
            scheduleAt(simTime() + HighDensityRecalculateFrequency, TLEvent);
        }
    }
    else if(msg->isName("tl switch evt"))   //Operations in sync with normal phase switching happens here
    {
        if(!UseHighDensityLogic and (currentPhase % 2 == 0) and LowDensityRecalculate())    //If using the low-density algorithm, and there's a vehicle that will benefit from it, delay the switch
        {
            TLSwitchEvent = new cMessage("tl switch evt");
            scheduleAt(simTime().dbl() + LowDensityExtendTime, TLSwitchEvent);  //Schedule the next switch after this duration
        }
        else    //Otherwise, switch to the next phase immediately
        {
            currentPhase = (currentPhase + 1) % phases.size();  //Switch to the next phase
            TraCI->commandSetPhase(id, currentPhase);           //And manually set this in SUMO
            TLSwitchEvent = new cMessage("tl switch evt");
            scheduleAt(simTime().dbl() + phases[currentPhase]->duration, TLSwitchEvent);  //Schedule the next switch after this duration
        }
        TraCI->commandSetPhaseDurationRemaining(id, 10000000);   //Make sure SUMO doesn't handle switches, by always setting phases to expire after a very long time
    }
}

void TrafficLight::HighDensityRecalculate()
{
    vector<Edge*>& edges = net->nodes[id]->inEdges;
    double phaseVehicleCounts[phases.size()];   //Will be the # of incoming vehicles that can go during that phase
    for(int i = 0; i < phases.size(); i++)  //Initialize to 0
        phaseVehicleCounts[i] = 0;
    int total = 0;

    for(vector<Edge*>::iterator edge = edges.begin(); edge != edges.end(); edge++)  //For each edge
    {
        vector<Lane*>* lanes = (*edge)->lanes;
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
        cout << "Recalculating TL " << id << ", currently in phase " << currentPhase << endl;

        for(int i = 0; i < phases.size(); i++)  //For each phase
        {
            if(i % 2 == 0)  //Ignore the odd (transitional) phases
            {
                double portion = (phaseVehicleCounts[i]/total) * 2;         //The portion of time allotted to that lane should be how many vehicles
                double duration = portion * nonTransitionalCycleDuration;   //can move during that phase divided by the total number of vehicles
                if(duration < 3)    //If the duration is too short, set it to a minimum
                    duration = 3;

                cout << "    Phase " << i << " set to " << duration << endl;
                phases[i]->duration = duration; //Update durations. These will take affect starting with the next phase
            }
        }
    }
}

bool TrafficLight::LowDensityRecalculate()    //This function assumes it's always LowDensityExtendTime away from the current phase ending,
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

    vector<Edge*>& edges = net->nodes[id]->inEdges; //Get all edges going into the TL
    for(vector<Edge*>::iterator edge = edges.begin(); edge != edges.end(); edge++)  //For each edge
    {
        vector<Lane*>* lanes = (*edge)->lanes; //Get all lanes on each edge
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
                            /*DEBUG
                            double remaining;
                            currentPhaseAtTime(simTime().dbl(), &remaining);
                            remaining *= 1000;
                            //double remaining = TraCI->commandGetNextSwitchTime(id) - simTime().dbl()*1000;// + 1000;
                            double extend = remaining + LowDensityExtendTime * 1000;
                            cout << id << " remaining is " << remaining << "ms, extending time to " << extend << "ms" << endl;*/
                            return true;    //We have found a vehicle that will benefit from extending
                        }
                    }
                }
            }
        }
    }
    return false;
}

void TrafficLight::finish()
{
    if(UseTLLogic)
    {
        if(UseHighDensityLogic)
        {
            cancelAndDelete(TLEvent);
        }
        //cancelAndDelete(TLSwitchEvent);

        /*
        if (TLEvent->isScheduled())
        {
            cancelAndDelete(TLEvent);
        }
        else
        {
            delete TLEvent;
        }*/
        if (TLSwitchEvent->isScheduled())
        {
            cancelAndDelete(TLSwitchEvent);
        }
        else
        {
            delete TLSwitchEvent;
        }
    }
}

void TrafficLight::print() // Print a node
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

int TrafficLight::currentPhaseAtTime(double time, double* timeRemaining)
{
    int phase = currentPhase;
    int curTime = lastSwitchTime + phases[phase]->duration; //Start at the next switch
    while(time >= curTime)  //While the time requested is less than curTime. When this breaks, our phase is set to the current phase
    {
        phase = (phase + 1) % phases.size();  //Move to the next phase
        curTime += phases[phase]->duration; //And add that duration to curTime
    }
    if(timeRemaining)   //If timeRemaining was passed in
    {
        *timeRemaining = curTime - time - 1;    //Calculate timeRemaining. This is off-by-one, so subtract one
        if(*timeRemaining <= 0) //If timeRemaining is zero, we've switched into the next phase, so add that phase's duration
        {
            *timeRemaining += phases[(phase + 1) % phases.size()]->duration;
        }
    }
    return phase;
}


}


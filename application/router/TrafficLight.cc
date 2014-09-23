#include "TrafficLight.h"

namespace VENTOS {

Define_Module(VENTOS::TrafficLight);

Phase::Phase(double duration, string state):
        duration(duration), state(state){}

ostream& operator<<(ostream& os, Phase &rhs) // Print a phase
{
    os << "duration: "<< setw(4) << left << rhs.duration <<
         " phase: " << setw(12) << left << rhs.state;
    return os;
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

    cycleDuration = 0;
    nonTransitionalCycleDuration = 0;
    int i = 0;
    for(vector<Phase*>::iterator it = phases.begin(); it != phases.end(); it++)
    {
        cycleDuration += (*it)->duration;
        if(i++ % 2 == 0)
            nonTransitionalCycleDuration += (*it)->duration;
    }

    nextSwitchTime = phases[0]->duration + cycleDuration;
    phaseBeforeSwitch = 0;
}

int TrafficLight::toPhase(int i)
{
    return i % phases.size();
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
            TLEvent = new cMessage("tl evt");   //Create a new internal message
            if (UseHighDensityLogic)
                scheduleAt(simTime() + HighDensityRecalculateFrequency, TLEvent); //Schedule them to start sending
            else    //Low Density Logic
                scheduleAt(1, TLEvent); //At 1s, TraCI communication is accurate and available, so we start then
        }
    }
}

void TrafficLight::handleMessage(cMessage* msg)  //Internal messages to self
{
    //if(id == "1/1")
    cout << id << " got message at " << simTime().dbl() << endl;


    TLEvent = new cMessage("tl evt");   //Create a new internal message
    if (UseHighDensityLogic)
    {
        HighDensityRecalculate();
        scheduleAt(simTime() + HighDensityRecalculateFrequency, TLEvent); //Schedule them to start sending
    }
    else //Low density logic
    {
        if(simTime().dbl() == 1)
            scheduleAt(phases[0]->duration - LowDensityExtendTime, TLEvent);
            //scheduleAt(TraCI->commandGetNextSwitchTime(id)/1000 - LowDensityExtendTime, TLEvent);
        else
        {
            if(LowDensityRecalculate()) //Phase duration was changed:
                scheduleAt(simTime().dbl() + LowDensityExtendTime, TLEvent);    //Wait LowDensityExtendTime, because that's what we added
            else
            {   //Phase duration was not changed:
                double nextEvent = simTime().dbl();
                int curPhase = TraCI->commandGetCurrentPhase(id);           //The current phase number
                nextEvent += phases[toPhase(curPhase + 1)]->duration;
                nextEvent += phases[toPhase(curPhase + 2)]->duration;   //Add the time of the next phases (transition and next phase)
                scheduleAt(nextEvent, TLEvent);
            }
        }
    }
}

void TrafficLight::HighDensityRecalculate()
{
    vector<Edge*>& edges = net->nodes[id]->inEdges;
    int phaseVehicleCounts[phases.size()];
    for(int i = 0; i < phases.size(); i++)
        phaseVehicleCounts[i] = 0;
    int total = 0;

    for(vector<Edge*>::iterator edge = edges.begin(); edge != edges.end(); edge++)
    {
        vector<Lane*>* lanes = (*edge)->lanes;
        for(vector<Lane*>::iterator lane = lanes->begin(); lane != lanes->end(); lane++)
        {
            list<string> vehicleIDs = TraCI->commandGetVehiclesOnLane((*lane)->id);
            int vehCount = vehicleIDs.size();
            for(vector<int>::iterator it = (*lane)->greenPhases.begin(); it != (*lane)->greenPhases.end(); it++)
            {
                phaseVehicleCounts[*it] += vehCount;
                total += vehCount;
                    //If we  want each vehicle to contribute exactly 1 weight, add vehCount/greenPhases.size() instead.
            }
        }
    }

    for(int i = 0; i < phases.size(); i++)
    {
        double portion = phaseVehicleCounts[i]/total;
        double duration = portion * nonTransitionalCycleDuration;

        phases[i]->duration = duration;

        //Make a call here
    }
    //Calculate the proportions of vehicles on each incoming edge, adjust phase times accordingly
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

    vector<Edge*>& edges = net->nodes[id]->inEdges;
    for(vector<Edge*>::iterator edge = edges.begin(); edge != edges.end(); edge++)
    {
        vector<Lane*>* lanes = (*edge)->lanes;
        for(vector<Lane*>::iterator lane = lanes->begin(); lane != lanes->end(); lane++)
        {
            list<string> vehicleIDs = TraCI->commandGetVehiclesOnLane((*lane)->id);
            for(list<string>::iterator vehicle = vehicleIDs.begin(); vehicle != vehicleIDs.end(); vehicle++)
            {
                double pos = TraCI->commandGetLanePosition(*vehicle);
                double length = net->edges[(*edge)->id]->length;
                double speed = net->edges[(*edge)->id]->speed;
                if((length - pos) / speed < 3.0)
                {
                    //Router::changeTLPhaseDuration(this, );
                    //Extend here
                    return true;
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
        if (TLEvent->isScheduled())
        {
            cancelAndDelete(TLEvent);
        }
        else
        {
            delete TLEvent;
        }
    }
}

ostream& operator<<(ostream& os, TrafficLight &rhs) // Print a node
{
    os << "id: "<< setw(4) << left << rhs.id <<
          "type: " << left << rhs.type <<
          "  programID: "<< setw(4) << left << rhs.programID <<
          "offset: "<< setw(4) << left << rhs.offset;
    for(vector<Phase*>::iterator it = rhs.phases.begin(); it != rhs.phases.end(); it++)
    {
        os << endl << **it;
    }
    return os;
}

}


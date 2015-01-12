#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include "BaseApplLayer.h"
#include "Global_01_TraCI_Extend.h"
#include "Net.h"
#include <vector>

using namespace std;

namespace VENTOS {

class TraCI_Extend;
class Net;

class Phase
{
public:
    double duration;
    string state;
    Phase(double durationVal, string stateVal);
    void print();
};

class TrafficLight : public cSimpleModule
{
public:
    string id;
    string type;
    string programID;
    double offset;
    vector<Phase*> phases;
    Node* node;
    Net* net;

    void build(string id, string type, string programID, double offset, vector<Phase*>& phases, Net* net);

    //Routing
    double lastSwitchTime;
    int currentPhase;
    int currentPhaseAtTime(double time, double* timeRemaining = NULL);

    //TL Control
    bool done;
    int TLLogicMode;
    double HighDensityRecalculateFrequency;
    double LowDensityExtendTime;
    double MaxPhaseDuration;

    double cycleDuration;   // this and below should be const
    double nonTransitionalCycleDuration;
    void HighDensityRecalculate();
    bool LowDensityRecalculate();

    //OmNET
    cMessage* TLEvent;
    cMessage* TLSwitchEvent;
    TraCI_Extend *TraCI;
    virtual void handleMessage(cMessage* msg);  //Internal messages to self;
    void initialize(int stage);
    void finish();
    TrafficLight();
    void print();
};


}

#endif

#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <vector>
#include <iostream>
#include <iomanip>

#include "BaseApplLayer.h"
#include "Net.h"
#include "Global_01_TraCI_Extend.h"

using namespace std;

namespace VENTOS {

class Phase
{
public:
    double duration;
    string state;
    Phase(double durationVal, string stateVal);
};
ostream& operator<<(ostream& os, Phase &rhs);

class Net;

class TrafficLight : public cSimpleModule
{
public:
    double nextSwitchTime;
    int phaseBeforeSwitch;

    string id;
    string type;
    string programID;
    double offset;
    vector<Phase*> phases;
    Node* node;
    Net* net;
    double cycleDuration;   // this and below should be const
    double nonTransitionalCycleDuration;
    void build(string id, string type, string programID, double offset, vector<Phase*>& phases, Net* net);
    inline int toPhase(int i);

    void HighDensityRecalculate();
    bool LowDensityRecalculate();

    //Message-passing
    bool UseTLLogic;
    bool UseHighDensityLogic;
    double HighDensityRecalculateFrequency;
    double LowDensityExtendTime;
    cMessage* TLEvent;
    cMessage* TLSwitchEvent;
    TraCI_Extend *TraCI;
    virtual void handleMessage(cMessage* msg);  //Internal messages to self;
    void initialize(int stage);
    void finish();
    TrafficLight();
};

ostream& operator<<(ostream& os, TrafficLight &rhs);

}

#endif

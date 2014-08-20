#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <vector>
#include <iostream>
#include <iomanip>

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

class TrafficLight
{
public:
    double nextSwitchTime;
    int phaseBeforeSwitch;

    string id;
    string type;
    string programID;
    double offset;
    vector<Phase*>* phases;
    double cycleDuration;
    TrafficLight(string idVal, string typeVal, string programIDval, double offsetVal, vector<Phase*>* phasesVec);
};

ostream& operator<<(ostream& os, TrafficLight &rhs);

}

#endif

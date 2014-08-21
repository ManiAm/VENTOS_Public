#include "TrafficLight.h"

namespace VENTOS {

Phase::Phase(double durationVal, string stateVal)
{
    duration = durationVal;
    state = stateVal;
}

ostream& operator<<(ostream& os, Phase &rhs) // Print a node
{
    os << "duration: "<< setw(4) << left << rhs.duration <<
         " phase: " << setw(12) << left << rhs.state;
    return os;
}

TrafficLight::TrafficLight(string idVal, string typeVal, string programIDval, double offsetVal, vector<Phase*>* phasesVec)
{
    id = idVal;
    type = typeVal;
    programID = programIDval;
    offset = offsetVal;
    phases = phasesVec;
    cycleDuration = 0;
    for(vector<Phase*>::iterator it = phasesVec->begin(); it != phasesVec->end(); it++)
        cycleDuration += (*it)->duration;

    nextSwitchTime = phases->at(0)->duration + cycleDuration;
    phaseBeforeSwitch = 0;
}

ostream& operator<<(ostream& os, TrafficLight &rhs) // Print a node
{
    os << "id: "<< setw(4) << left << rhs.id <<
          "type: " << left << rhs.type <<
          "  programID: "<< setw(4) << left << rhs.programID <<
          "offset: "<< setw(4) << left << rhs.offset;
    for(vector<Phase*>::iterator it = rhs.phases->begin(); it != rhs.phases->end(); it++)
    {
        os << endl << **it;
    }
    return os;
}

}


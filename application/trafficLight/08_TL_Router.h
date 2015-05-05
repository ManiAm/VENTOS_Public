/****************************************************************************/
/// @file    TrafficLightRouter.h
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

#ifndef TRAFFICLIGHTROUTER_H
#define TRAFFICLIGHTROUTER_H

#include <07_TL_VANET.h>
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

class Router;

class TrafficLightRouter : public TrafficLightVANET
{
public:
    TrafficLightRouter();
    virtual void finish();
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *);


public:
    string id;
    string type;
    string programID;
    double offset;
    vector<Phase*> phases;
    Node* node;
    Net* net;

    //OmNET
    cMessage* TLEvent;
    cMessage* TLSwitchEvent;
    TraCI_Extend *TraCI;
    void print();

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
    double MinPhaseDuration;

    double cycleDuration;   // this and below should be const
    double nonTransitionalCycleDuration;

    void switchToPhase(int phase, double greenDuration = -1, int yellowDuration = 3);
    void ASynchronousMessage();
    void SynchronousMessage();
    void FlowRateRecalculate();
    void HighDensityRecalculate();
    bool LowDensityVehicleCheck();
    void LowDensityRecalculate();

private:
    Router* router;
    bool isTransitionPhase;
    int phaseDurationAfterTransition;
};

}

#endif

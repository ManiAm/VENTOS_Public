/****************************************************************************/
/// @file    TL_Webster.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
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

#include <06_TL_AdaptiveWebster.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightWebster);


TrafficLightWebster::~TrafficLightWebster()
{

}


void TrafficLightWebster::initialize(int stage)
{
    TrafficLightAdaptiveQueue::initialize(stage);

    if(TLControlMode != 4)
        return;

    if(stage == 0)
    {
        minGreenTime = par("minGreenTime").doubleValue();
        maxGreenTime = par("maxGreenTime").doubleValue();
        yellowTime = par("yellowTime").doubleValue();
        redTime = par("redTime").doubleValue();

        // set initial values
        intervalOffSet = minGreenTime;
        intervalElapseTime = 0.0;
        currentInterval = phase1_5;

        ChangeEvt = new cMessage("ChangeEvt", 1);
        scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);
    }
}


void TrafficLightWebster::finish()
{
    TrafficLightAdaptiveQueue::finish();

}


void TrafficLightWebster::handleMessage(cMessage *msg)
{
    TrafficLightAdaptiveQueue::handleMessage(msg);

    if(TLControlMode != 4)
        return;

    if (msg == ChangeEvt)
    {
        chooseNextInterval();

        // Schedule next light change event:
        scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);
    }
}


void TrafficLightWebster::executeFirstTimeStep()
{
    TrafficLightAdaptiveQueue::executeFirstTimeStep();

    if(TLControlMode != 4)
        return;

    cout << "Dynamic Webster traffic signal control ... " << endl << endl;

    for (list<string>::iterator TL = TLList.begin(); TL != TLList.end(); TL++)
    {
        TraCI->TLSetProgram(*TL, "adaptive-time");
        TraCI->TLSetState(*TL, phase1_5);
    }

    char buff[300];
    sprintf(buff, "Sim time: %4.2f | Interval finish time: %4.2f | Current interval: %s", simTime().dbl(), simTime().dbl() + intervalOffSet, currentInterval.c_str() );
    cout << buff << endl;
}


void TrafficLightWebster::executeEachTimeStep(bool simulationDone)
{
    TrafficLightAdaptiveQueue::executeEachTimeStep(simulationDone);
}


void TrafficLightWebster::chooseNextInterval()
{
    intervalElapseTime += intervalOffSet;

    if (currentInterval == "yellow")
    {
        currentInterval = "red";

        // change all 'y' to 'r'
        string str = TraCI->TLGetState("C");
        string nextInterval = "";
        for(char& c : str) {
            if (c == 'y')
                nextInterval += 'r';
            else
                nextInterval += c;
        }

        // set the new state
        TraCI->TLSetState("C", nextInterval);
        intervalElapseTime = 0.0;
        intervalOffSet = redTime;
    }
    else if (currentInterval == "red")
    {
        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState("C", nextGreenInterval);
        intervalElapseTime = 0.0;
        intervalOffSet = minGreenTime;
    }
    else
        chooseNextGreenInterval();

    char buff[300];
    sprintf(buff, "Sim time: %4.2f | Interval finish time: %4.2f | Current interval: %s", simTime().dbl(), simTime().dbl() + intervalOffSet, currentInterval.c_str() );
    cout << buff << endl;
}


void TrafficLightWebster::chooseNextGreenInterval()
{
    list<string> lan = TraCI->TLGetControlledLanes("C");

    // remove duplicate entries
    lan.unique();

    int max = -1;
    string criticalLane = "";

    // for each incoming lane
    for(list<string>::iterator it = lan.begin(); it != lan.end(); ++it)
    {
        // todo: change this to traffic demand rather than queue size
        int qSize = laneQueueSize[*it].second;
        if( qSize > max)
        {
            max = qSize;
            criticalLane = *it;
        }
    }

    cout << simTime().dbl() << ": critical lane is " << criticalLane << endl;


}

}

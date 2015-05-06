/****************************************************************************/
/// @file    TL_AdaptiveQueue.cc
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

#include <05_TL_AdaptiveQueue.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightAdaptiveQueue);


TrafficLightAdaptiveQueue::~TrafficLightAdaptiveQueue()
{

}


void TrafficLightAdaptiveQueue::initialize(int stage)
{
    TrafficLightAdaptive::initialize(stage);

    if(TLControlMode != 3)
        return;

    if(stage == 0)
    {
        minGreenTime = par("minGreenTime").doubleValue();
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


void TrafficLightAdaptiveQueue::finish()
{
    TrafficLightAdaptive::finish();

}


void TrafficLightAdaptiveQueue::handleMessage(cMessage *msg)
{
    TrafficLightAdaptive::handleMessage(msg);

    if(TLControlMode != 3)
        return;

    if (msg == ChangeEvt)
    {
        chooseNextInterval();

        // Schedule next light change event:
        scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);
    }
}


void TrafficLightAdaptiveQueue::executeFirstTimeStep()
{
    TrafficLightAdaptive::executeFirstTimeStep();

    if(TLControlMode != 3)
        return;

    cout << "Adaptive-time with queue traffic signal control ..." << endl << endl;

    for (list<string>::iterator TL = TLList.begin(); TL != TLList.end(); TL++)
    {
        TraCI->TLSetProgram(*TL, "adaptive-time");
        TraCI->TLSetState(*TL, phase1_5);
    }

    char buff[300];
    sprintf(buff, "Sim time: %4.2f | Interval finish time: %4.2f | Current interval: %s", simTime().dbl(), simTime().dbl() + intervalOffSet, currentInterval.c_str() );
    cout << buff << endl;
}


void TrafficLightAdaptiveQueue::executeEachTimeStep(bool simulationDone)
{
    TrafficLightAdaptive::executeEachTimeStep(simulationDone);
}


void TrafficLightAdaptiveQueue::chooseNextInterval()
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
    {
        chooseNextGreenInterval();
    }

    char buff[300];
    sprintf(buff, "Sim time: %4.2f | Interval finish time: %4.2f | Current interval: %s", simTime().dbl(), simTime().dbl() + intervalOffSet, currentInterval.c_str() );
    cout << buff << endl;
}


void TrafficLightAdaptiveQueue::chooseNextGreenInterval()
{
    int maxQueue = -1;
    int row = -1;

    // get which row has the highest queue length
    for(int i = 0; i < LINKSIZE; i++)  // row
    {
        int totalQueueRow = 0;
        totalQueueRow = totalQueueRow + queueSizeLink("C", i);

        for(int j = i+1; j < LINKSIZE; j++)  // column
        {
            // if link i and link j are not conflicting
            if(nonConflictingLinks[i][j] == 1)
                totalQueueRow = totalQueueRow + queueSizeLink("C", j);
        }

        if(totalQueueRow > maxQueue)
        {
            maxQueue = totalQueueRow;
            row = i;
        }
    }

    if(maxQueue == -1 || row == -1)
        error("something is wrong!");

    // Calculate the next green interval
    nextGreenInterval = "";
    for(int j = 0; j < LINKSIZE; j++)
    {
        if( nonConflictingLinks[row][j] == 1 )
            nextGreenInterval += 'G';
        else
            nextGreenInterval += 'r';
    }
    nextGreenInterval[row] = 'G';

    // Calculate the next interval
    string nextInterval = "";
    bool needYellowInterval = false;  // if we have at least one yellow interval
    for(int i = 0; i < LINKSIZE; i++)
    {
        if( (currentInterval[i] == 'G' || currentInterval[i] == 'g') && nextGreenInterval[i] == 'r')
        {
            nextInterval += 'y';
            needYellowInterval = true;
        }
        else
            nextInterval += currentInterval[i];
    }

    cout << endl << "servicing lanes     " << row << ", ";
    for(int k =0; k < LINKSIZE; k++)
        if(nonConflictingLinks[row][k] == 1)
            cout << k << ", ";
    cout << endl;
    cout << "total queue size    " << maxQueue << endl;
    cout << "current interval    " << currentInterval << endl;
    cout << "next green interval " << nextGreenInterval << endl;
    cout << "next interval       " << nextInterval << endl;

    if(needYellowInterval)
    {
        currentInterval = "yellow";
        TraCI->TLSetState("C", nextInterval);

        intervalElapseTime = 0.0;
        intervalOffSet =  yellowTime;
    }
    else
    {
        intervalOffSet = minGreenTime;
        cout << "Continue the last green interval." << endl;
    }
}


int TrafficLightAdaptiveQueue::queueSizeLink(string TLid, int link)
{
    map<int,string> result = TraCI->TLGetControlledLinks(TLid);

    string incommingLane;
    char_separator<char> sep("|");
    tokenizer< char_separator<char> > tokens(result[link], sep);

    for(tokenizer< char_separator<char> >::iterator beg=tokens.begin(); beg!=tokens.end(); ++beg)
    {
        incommingLane = (*beg).c_str();
        break;
    }

    return laneQueueSize[incommingLane].second;
}

}

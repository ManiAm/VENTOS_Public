/****************************************************************************/
/// @file    TL_Adaptive.cc
/// @author  Philip Vo <foxvo@ucdavis.edu>
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

#include <04_TL_Adaptive.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightAdaptive);


TrafficLightAdaptive::~TrafficLightAdaptive()
{

}


void TrafficLightAdaptive::initialize(int stage)
{
    TrafficLightFixed::initialize(stage);

    if(TLControlMode != 2)
        return;

    if(stage == 0)
    {
        minGreenTime = par("minGreenTime").doubleValue();
        maxGreenTime = par("maxGreenTime").doubleValue();
        yellowTime = par("yellowTime").doubleValue();
        redTime = par("redTime").doubleValue();
        passageTime = par("passageTime").doubleValue();

        // set initial values
        intervalOffSet = minGreenTime;
        intervalElapseTime = 0.0;
        currentInterval = phase1_5;

        ChangeEvt = new cMessage("ChangeEvt", 1);
        scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);

        cout << endl << "Adaptive-time traffic signal control ..."  << endl << endl;
    }
}


void TrafficLightAdaptive::finish()
{
    TrafficLightFixed::finish();

}


void TrafficLightAdaptive::handleMessage(cMessage *msg)
{
    TrafficLightFixed::handleMessage(msg);

    if(TLControlMode != 2)
        return;

    if (msg == ChangeEvt)
    {
        chooseNextInterval();

        // Schedule next light change event:
        scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);
    }
}


void TrafficLightAdaptive::executeFirstTimeStep()
{
    // call parent
    TrafficLightFixed::executeFirstTimeStep();

    // set the program and initial TL state in all TLs
    if(TLControlMode == 2)
    {
        for (list<string>::iterator TL = TLList.begin(); TL != TLList.end(); TL++)
        {
            TraCI->commandSetTLProgram(*TL, "adaptive-time");
            TraCI->commandSetTLState(*TL, phase1_5);
        }

        char buff[300];
        sprintf(buff, "Sim time: %4.2f | Interval finish time: %4.2f | Current interval: %s", simTime().dbl(), simTime().dbl() + intervalOffSet, currentInterval.c_str() );
        cout << buff << endl;
    }
}


void TrafficLightAdaptive::executeEachTimeStep(bool simulationDone)
{
    // call parent
    TrafficLightFixed::executeEachTimeStep(simulationDone);
}


void TrafficLightAdaptive::chooseNextInterval()
{
    intervalElapseTime += intervalOffSet;

    if (currentInterval == "yellow")
    {
        currentInterval = "red";

        // change all 'y' to 'r'
        string str = TraCI->commandGetTLState("C");
        string nextInterval = "";
        for(char& c : str) {
            if (c == 'y')
                nextInterval += 'r';
            else
                nextInterval += c;
        }

        // set the new state
        TraCI->commandSetTLState("C", nextInterval);
        intervalElapseTime = 0.0;
        intervalOffSet = redTime;
    }
    else if (currentInterval == "red")
    {
        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->commandSetTLState("C", nextGreenInterval);
        intervalElapseTime = 0.0;
        intervalOffSet = minGreenTime;
    }
    else
        chooseNextGreenInterval();

    char buff[300];
    sprintf(buff, "Sim time: %4.2f | Interval finish time: %4.2f | Current interval: %s", simTime().dbl(), simTime().dbl() + intervalOffSet, currentInterval.c_str() );
    cout << buff << endl;
}


void TrafficLightAdaptive::chooseNextGreenInterval()
{
    // get loop detector information
    vector<double> LastActuatedTime;

    // If maxGreenTime met, don't care for actuator values,
    // so push passageTime so that conditions fail and move
    // to red interval:
    if (intervalElapseTime >= maxGreenTime)
        for (list<string>::iterator LD = LDList.begin(); LD != LDList.end(); LD++)
            LastActuatedTime.push_back(passageTime);
    else
        for (list<string>::iterator LD = LDList.begin(); LD != LDList.end(); LD++)
            LastActuatedTime.push_back(TraCI->commandGetLoopDetectorLastTime(*LD));

    // Do proper transition:
    if (currentInterval == phase1_5)
    {
        if (LastActuatedTime[NC_4] < passageTime && LastActuatedTime[SC_4] < passageTime)
        {
            double smallest = ((LastActuatedTime[NC_4] < LastActuatedTime[SC_4]) ? LastActuatedTime[NC_4] : LastActuatedTime[SC_4]);
            intervalOffSet = ceil(passageTime - smallest);

            double extendTime = intervalElapseTime + intervalOffSet;
            // Never extend past maxGreenTime:
            if (extendTime > maxGreenTime)
                intervalOffSet = intervalOffSet - (extendTime - maxGreenTime);

            cout << "Extending green time by: " << intervalOffSet << "s" << endl;
        }
        else if (LastActuatedTime[NC_4] < passageTime)
        {
            nextGreenInterval = phase2_5;
            string nextInterval = "rrrrGrrrrrrrrryrrrrrrrrr";

            currentInterval = "yellow";
            TraCI->commandSetTLState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
        else if (LastActuatedTime[SC_4] < passageTime)
        {
            nextGreenInterval = phase1_6;
            string nextInterval = "rrrryrrrrrrrrrGrrrrrrrrr";

            currentInterval = "yellow";
            TraCI->commandSetTLState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
        else
        {
            nextGreenInterval = phase2_6;
            string nextInterval = "rrrryrrrrrrrrryrrrrrrrrr";

            currentInterval = "yellow";
            TraCI->commandSetTLState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
    }
    else if (currentInterval == phase2_5)
    {
        if (LastActuatedTime[NC_4] < passageTime)
        {
            intervalOffSet = ceil(passageTime - LastActuatedTime[NC_4]);

            double extendTime = intervalElapseTime + intervalOffSet;
            // Never extend past maxGreenTime:
            if (extendTime > maxGreenTime)
                intervalOffSet = intervalOffSet - (extendTime - maxGreenTime);

            cout << "Extending green time by: " << intervalOffSet << "s" << endl;
        }
        else
        {
            nextGreenInterval = phase2_6;
            string nextInterval = "gGgGyrrrrrrrrrrrrrrrrrrG";

            currentInterval = "yellow";
            TraCI->commandSetTLState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
    }
    else if (currentInterval == phase1_6)
    {
        if (LastActuatedTime[SC_4] < passageTime)
        {
            intervalOffSet = ceil(passageTime - LastActuatedTime[SC_4]);

            double extendTime = intervalElapseTime + intervalOffSet;
            // Never extend past maxGreenTime:
            if (extendTime > maxGreenTime)
                intervalOffSet = intervalOffSet - (extendTime - maxGreenTime);

            cout << "Extending green time by: " << intervalOffSet << "s" << endl;
        }
        else
        {
            nextGreenInterval = phase2_6;
            string nextInterval = "rrrrrrrrrrgGgGyrrrrrrGrr";

            currentInterval = "yellow";
            TraCI->commandSetTLState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
    }
    else if (currentInterval == phase2_6)
    {
        if (LastActuatedTime[NC_2] < passageTime || LastActuatedTime[NC_3] < passageTime ||
                LastActuatedTime[SC_2] < passageTime || LastActuatedTime[SC_3] < passageTime)
        {
            double smallest1 = ((LastActuatedTime[NC_2] < LastActuatedTime[SC_2]) ? LastActuatedTime[NC_2] : LastActuatedTime[SC_2]);
            double smallest2 = ((LastActuatedTime[NC_3] < LastActuatedTime[SC_3]) ? LastActuatedTime[NC_3] : LastActuatedTime[SC_3]);
            double smallest = ((smallest1 < smallest2) ? smallest1 : smallest2);
            intervalOffSet = ceil(passageTime - smallest);

            double extendTime = intervalElapseTime + intervalOffSet;
            // Never extend past maxGreenTime:
            if (extendTime > maxGreenTime)
                intervalOffSet = intervalOffSet - (extendTime - maxGreenTime);

            cout << "Extending green time by: " << intervalOffSet << "s" << endl;
        }
        else
        {
            nextGreenInterval = phase3_7;
            string nextInterval = "yyyyrrrrrryyyyrrrrrrryry";

            currentInterval = "yellow";
            TraCI->commandSetTLState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
    }
    else if (currentInterval == phase3_7)
    {
        if (LastActuatedTime[WC_4] < passageTime && LastActuatedTime[EC_4] < passageTime)
        {
            double smallest = ((LastActuatedTime[WC_4] < LastActuatedTime[EC_4]) ? LastActuatedTime[WC_4] : LastActuatedTime[EC_4]);
            intervalOffSet = ceil(passageTime - smallest);

            double extendTime = intervalElapseTime + intervalOffSet;
            // Never extend past maxGreenTime:
            if (extendTime > maxGreenTime)
                intervalOffSet = intervalOffSet - (extendTime - maxGreenTime);

            cout << "Extending green time by: " << intervalOffSet << "s" << endl;
        }
        else if (LastActuatedTime[WC_4] < passageTime)
        {
            nextGreenInterval = phase3_8;
            string nextInterval = "rrrrrrrrryrrrrrrrrrGrrrr";

            currentInterval = "yellow";
            TraCI->commandSetTLState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
        else if (LastActuatedTime[EC_4] < passageTime)
        {
            nextGreenInterval = phase4_7;
            string nextInterval = "rrrrrrrrrGrrrrrrrrryrrrr";

            currentInterval = "yellow";
            TraCI->commandSetTLState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
        else
        {
            nextGreenInterval = phase4_8;
            string nextInterval = "rrrrrrrrryrrrrrrrrryrrrr";

            currentInterval = "yellow";
            TraCI->commandSetTLState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
    }
    else if (currentInterval == phase3_8)
    {
        if (LastActuatedTime[WC_4] < passageTime)
        {
            intervalOffSet = ceil(passageTime - LastActuatedTime[WC_4]);

            double extendTime = intervalElapseTime + intervalOffSet;
            // Never extend past maxGreenTime:
            if (extendTime > maxGreenTime)
                intervalOffSet = intervalOffSet - (extendTime - maxGreenTime);

            cout << "Extending green time by: " << intervalOffSet << "s" << endl;
        }
        else
        {
            nextGreenInterval = phase4_8;
            string nextInterval = "rrrrrrrrrrrrrrrgGgGyrrGr";

            currentInterval = "yellow";
            TraCI->commandSetTLState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
    }
    else if (currentInterval == phase4_7)
    {
        if (LastActuatedTime[EC_4] < passageTime)
        {
            intervalOffSet = ceil(passageTime - LastActuatedTime[EC_4]);

            double extendTime = intervalElapseTime + intervalOffSet;
            // Never extend past maxGreenTime:
            if (extendTime > maxGreenTime)
                intervalOffSet = intervalOffSet - (extendTime - maxGreenTime);

            cout << "Extending green time by: " << intervalOffSet << "s" << endl;
        }
        else
        {
            nextGreenInterval = phase4_8;
            string nextInterval = "rrrrrgGgGyrrrrrrrrrrGrrr";

            currentInterval = "yellow";
            TraCI->commandSetTLState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
    }
    else if (currentInterval == phase4_8)
    {
        if (LastActuatedTime[WC_2] < passageTime || LastActuatedTime[WC_3] < passageTime ||
                LastActuatedTime[EC_2] < passageTime || LastActuatedTime[EC_3] < passageTime)
        {
            double smallest1 = ((LastActuatedTime[WC_2] < LastActuatedTime[EC_2]) ? LastActuatedTime[WC_2] : LastActuatedTime[EC_2]);
            double smallest2 = ((LastActuatedTime[WC_3] < LastActuatedTime[EC_3]) ? LastActuatedTime[WC_3] : LastActuatedTime[EC_3]);
            double smallest = ((smallest1 < smallest2) ? smallest1 : smallest2);
            intervalOffSet = ceil(passageTime - smallest);

            double extendTime = intervalElapseTime + intervalOffSet;
            // Never extend past maxGreenTime:
            if (extendTime > maxGreenTime)
                intervalOffSet = intervalOffSet - (extendTime - maxGreenTime);

            cout << "Extending green time by: " << intervalOffSet << "s" << endl;
        }
        else
        {
            nextGreenInterval = phase1_5;
            string nextInterval = "rrrrryyyyrrrrrryyyyryryr";

            currentInterval = "yellow";
            TraCI->commandSetTLState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
    }
}

}

/****************************************************************************/
/// @file    TrafficLightControl.cc
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

#include <TrafficLightControl.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightControl);


TrafficLightControl::~TrafficLightControl()
{

}


void TrafficLightControl::initialize(int stage)
{
    TrafficLightBase::initialize(stage);

    if(stage == 0)
    {
        minGreenTime = par("minGreenTime").doubleValue();
        maxGreenTime = par("maxGreenTime").doubleValue();
        yellowTime = par("yellowTime").doubleValue();
        redTime = par("redTime").doubleValue();
        passageTime = par("passageTime").doubleValue();
        detectorPos = par("detectorPos").doubleValue();

        if(TLControlMode == 2 || TLControlMode == 3)
        {
            ChangeEvt = new cMessage("ChangeEvt", 1);

            offSet = minGreenTime;
            scheduleAt(simTime().dbl() + offSet, ChangeEvt);

            if (TLControlMode == 3)
            {
                DetectedTime.assign(lmap.size(), 0.0);
                DetectEvt = new cMessage("DetectEvt", 1);
                scheduleAt(simTime().dbl() + detectFreq, DetectEvt);
            }
        }
    }
}


void TrafficLightControl::finish()
{
    TrafficLightBase::finish();

}


void TrafficLightControl::handleMessage(cMessage *msg)
{
    TrafficLightBase::handleMessage(msg);

    if (msg == ChangeEvt)
    {
        intervalElapseTime += offSet;
        string curInterval = currentInterval;

        // Choose traffic light controller:
        if (TLControlMode == 2)
        {
            cout << endl << "Going into doAdaptiveTimeControl()" << endl;
            doAdaptiveTimeControl();
        }
        else if (TLControlMode == 3)
        {
            cout << endl << "Going into doVANETControl()" << endl;
            doVANETControl();
        }

        // If extending current green interval:
        if (curInterval == currentInterval)
        {
            double extendTime = offSet + intervalElapseTime;

            // Never extend past maxGreenTime:
            if (extendTime > maxGreenTime)
                offSet = offSet - (extendTime - maxGreenTime);

            cout << "Extending green time by: " << offSet << "s" << endl;
        }

        // Schedule next light change event:
        scheduleAt(simTime().dbl() + offSet, ChangeEvt);

        char buff[300];
        sprintf(buff, "sim time: %4.2f | Interval finish time: %4.2f | Interval elapsed time: %4.2f | Current interval: %s", simTime().dbl(), simTime().dbl() + offSet, intervalElapseTime, currentInterval.c_str() );
        cout << buff << endl;
    }
    else if (msg == DetectEvt)
    {
        list<string> VehList = TraCI->commandGetVehicleList();
        for(list<string>::iterator V = VehList.begin(); V != VehList.end(); V++)
        {
            Coord pos = TraCI->commandGetVehiclePos(*V);

            // If within radius of traffic light:
            if ((pos.x > 65 && pos.x < 69) || (pos.x > 131 && pos.x < 135) ||
                    (pos.y > 65 && pos.y < 69) || (pos.y > 131 && pos.y < 135))
            {
                // If in lane of interest (heading towards TL):
                string lane = TraCI->commandGetVehicleLaneId(*V);
                if (lane == "EC_2" || lane == "EC_3" || lane == "EC_4" ||
                        lane == "NC_2" || lane == "NC_3" || lane == "NC_4" ||
                        lane == "SC_2" || lane == "SC_3" || lane == "SC_4" ||
                        lane == "WC_2" || lane == "WC_3" || lane == "WC_4")
                {
                    DetectedTime[lmap[lane]] = simTime().dbl();
                }
            }
        }

        // Schedule next detection event:
        scheduleAt(simTime().dbl() + detectFreq, DetectEvt);
    }
}


void TrafficLightControl::executeFirstTimeStep()
{
    // call parent
    TrafficLightBase::executeFirstTimeStep();

    LDList = TraCI->commandGetLoopDetectorList();

    // fixed-time traffic signal
    if (TLControlMode == 1)
    {
        for (list<string>::iterator TL = TLList.begin(); TL != TLList.end(); TL++)
            TraCI->commandSetTLProgram(*TL, "fix-time");
    }
    // adaptive-time and VANET traffic signal
    else if(TLControlMode == 2 || TLControlMode == 3)
    {
        for (list<string>::iterator TL = TLList.begin(); TL != TLList.end(); TL++)
        {
            //TraCI->commandSetTLProgram(*TL, "adaptive-time");
            TraCI->commandSetTLState(*TL, phase1_5);
        }

        currentInterval = phase1_5;
    }
    else
    {
        error("Invalid TLControlMode!");
    }
}


void TrafficLightControl::executeEachTimeStep()
{
    // call parent
    TrafficLightBase::executeEachTimeStep();

    //    if (TLControlMode == 1 || TLControlMode == 2)
    //        return;
    //    else if (TLControlMode == 3)
    //    {
    //
    //    }
}


void TrafficLightControl::doAdaptiveTimeControl()
{
    string nextInterval;

    if (currentInterval == "yellow")
    {
        currentInterval = "red";

        // change all 'y' to 'r'
        string str = TraCI->commandGetTLState("C");
        for(char& c : str) {
            if (c == 'y')
                nextInterval += 'r';
            else
                nextInterval += c;
        }

        // set the new state
        TraCI->commandSetTLState("C", nextInterval);
        intervalElapseTime = 0.0;
        offSet = redTime;

        return;
    }
    else if (currentInterval == "red")
    {
        currentInterval = nextGreenInterval;
        TraCI->commandSetTLState("C", nextGreenInterval);
        intervalElapseTime = 0.0;
        offSet = minGreenTime;

        return;
    }

    // If we reach here, it means that we are in green interval.

    // First get loop detector information
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
            offSet = ceil(passageTime - smallest);
            return;
        }
        else if (LastActuatedTime[NC_4] < passageTime)
        {
            nextGreenInterval = phase2_5;
            nextInterval = "rrrrGrrrrrrrrryrrrrrrrrr";
        }
        else if (LastActuatedTime[SC_4] < passageTime)
        {
            nextGreenInterval = phase1_6;
            nextInterval = "rrrryrrrrrrrrrGrrrrrrrrr";
        }
        else
        {
            nextGreenInterval = phase2_6;
            nextInterval = "rrrryrrrrrrrrryrrrrrrrrr";
        }
    }
    else if (currentInterval == phase2_5)
    {
        if (LastActuatedTime[NC_4] < passageTime)
        {
            offSet = ceil(passageTime - LastActuatedTime[NC_4]);
            return;
        }
        else
        {
            nextGreenInterval = phase2_6;
            nextInterval = "gGgGyrrrrrrrrrrrrrrrrrrG";
        }
    }
    else if (currentInterval == phase1_6)
    {
        if (LastActuatedTime[SC_4] < passageTime)
        {
            offSet = ceil(passageTime - LastActuatedTime[SC_4]);
            return;
        }
        else
        {
            nextGreenInterval = phase2_6;
            nextInterval = "rrrrrrrrrrgGgGyrrrrrrGrr";
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
            offSet = ceil(passageTime - smallest);
            return;
        }
        else
        {
            nextGreenInterval = phase3_7;
            nextInterval = "yyyyrrrrrryyyyrrrrrrryry";
        }
    }
    else if (currentInterval == phase3_7)
    {
        if (LastActuatedTime[WC_4] < passageTime && LastActuatedTime[EC_4] < passageTime)
        {
            double smallest = ((LastActuatedTime[WC_4] < LastActuatedTime[EC_4]) ? LastActuatedTime[WC_4] : LastActuatedTime[EC_4]);
            offSet = ceil(passageTime - smallest);
            return;
        }
        else if (LastActuatedTime[WC_4] < passageTime)
        {
            nextGreenInterval = phase3_8;
            nextInterval = "rrrrrrrrryrrrrrrrrrGrrrr";
        }
        else if (LastActuatedTime[EC_4] < passageTime)
        {
            nextGreenInterval = phase4_7;
            nextInterval = "rrrrrrrrrGrrrrrrrrryrrrr";
        }
        else
        {
            nextGreenInterval = phase4_8;
            nextInterval = "rrrrrrrrryrrrrrrrrryrrrr";
        }
    }
    else if (currentInterval == phase3_8)
    {
        if (LastActuatedTime[WC_4] < passageTime)
        {
            offSet = ceil(passageTime - LastActuatedTime[WC_4]);
            return;
        }
        else
        {
            nextGreenInterval = phase4_8;
            nextInterval = "rrrrrrrrrrrrrrrgGgGyrrGr";
        }
    }
    else if (currentInterval == phase4_7)
    {
        if (LastActuatedTime[EC_4] < passageTime)
        {
            offSet = ceil(passageTime - LastActuatedTime[EC_4]);
            return;
        }
        else
        {
            nextGreenInterval = phase4_8;
            nextInterval = "rrrrrgGgGyrrrrrrrrrrGrrr";
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
            offSet = ceil(passageTime - smallest);
            return;
        }
        else
        {
            nextGreenInterval = phase1_5;
            nextInterval = "rrrrryyyyrrrrrryyyyryryr";
        }
    }

    currentInterval = "yellow";
    TraCI->commandSetTLState("C", nextInterval);
    intervalElapseTime = 0.0;
    offSet =  yellowTime;
}


void TrafficLightControl::doVANETControl()
{
    string nextInterval;

    if (currentInterval == "yellow")
    {
        currentInterval = "red";

        string str = TraCI->commandGetTLState("C");
        for(char& c : str) {
            if (c == 'y')
                nextInterval += 'r';
            else
                nextInterval += c;
        }

        TraCI->commandSetTLState("C", nextInterval);
        intervalElapseTime = 0.0;
        offSet = redTime;

        return;
    }
    else if (currentInterval == "red")
    {
        currentInterval = nextGreenInterval;
        TraCI->commandSetTLState("C", nextGreenInterval);
        intervalElapseTime = 0.0;
        offSet = minGreenTime;

        return;
    }

    // If we reach here, it means that we are in green interval.

    // First get adjusted VANET detector times
    vector<double> LastDetectedTime;

    for (vector<double>::iterator it = DetectedTime.begin(); it != DetectedTime.end(); it++)
        LastDetectedTime.push_back(simTime().dbl() - (*it));


    // Do proper transition:
    if (currentInterval == phase1_5)
    {
        if (LastDetectedTime[NC_4] < passageTime && LastDetectedTime[SC_4] < passageTime)
        {
            double smallest = ((LastDetectedTime[NC_4] < LastDetectedTime[SC_4]) ? LastDetectedTime[NC_4] : LastDetectedTime[SC_4]);
            offSet = ceil(passageTime - smallest);
            return;
        }
        else if (LastDetectedTime[NC_4] < passageTime)
        {
            nextGreenInterval = phase2_5;
            nextInterval = "rrrrGrrrrrrrrryrrrrrrrrr";
        }
        else if (LastDetectedTime[SC_4] < passageTime)
        {
            nextGreenInterval = phase1_6;
            nextInterval = "rrrryrrrrrrrrrGrrrrrrrrr";
        }
        else
        {
            nextGreenInterval = phase2_6;
            nextInterval = "rrrryrrrrrrrrryrrrrrrrrr";
        }
    }
    else if (currentInterval == phase2_5)
    {
        if (LastDetectedTime[NC_4] < passageTime)
        {
            offSet = ceil(passageTime - LastDetectedTime[NC_4]);
            return;
        }
        else
        {
            nextGreenInterval = phase2_6;
            nextInterval = "gGgGyrrrrrrrrrrrrrrrrrrG";
        }
    }
    else if (currentInterval == phase1_6)
    {
        if (LastDetectedTime[SC_4] < passageTime)
        {
            offSet = ceil(passageTime - LastDetectedTime[SC_4]);
            return;
        }
        else
        {
            nextGreenInterval = phase2_6;
            nextInterval = "rrrrrrrrrrgGgGyrrrrrrGrr";
        }
    }
    else if (currentInterval == phase2_6)
    {
        if (LastDetectedTime[NC_2] < passageTime || LastDetectedTime[NC_3] < passageTime ||
                LastDetectedTime[SC_2] < passageTime || LastDetectedTime[SC_3] < passageTime)
        {
            double smallest1 = ((LastDetectedTime[NC_2] < LastDetectedTime[SC_2]) ? LastDetectedTime[NC_2] : LastDetectedTime[SC_2]);
            double smallest2 = ((LastDetectedTime[NC_3] < LastDetectedTime[SC_3]) ? LastDetectedTime[NC_3] : LastDetectedTime[SC_3]);
            double smallest = ((smallest1 < smallest2) ? smallest1 : smallest2);
            offSet = ceil(passageTime - smallest);
            return;
        }
        else
        {
            nextGreenInterval = phase3_7;
            nextInterval = "yyyyrrrrrryyyyrrrrrrryry";
        }
    }
    else if (currentInterval == phase3_7)
    {
        if (LastDetectedTime[WC_4] < passageTime && LastDetectedTime[EC_4] < passageTime)
        {
            double smallest = ((LastDetectedTime[WC_4] < LastDetectedTime[EC_4]) ? LastDetectedTime[WC_4] : LastDetectedTime[EC_4]);
            offSet = ceil(passageTime - smallest);
            return;
        }
        else if (LastDetectedTime[WC_4] < passageTime)
        {
            nextGreenInterval = phase3_8;
            nextInterval = "rrrrrrrrryrrrrrrrrrGrrrr";
        }
        else if (LastDetectedTime[EC_4] < passageTime)
        {
            nextGreenInterval = phase4_7;
            nextInterval = "rrrrrrrrrGrrrrrrrrryrrrr";
        }
        else
        {
            nextGreenInterval = phase4_8;
            nextInterval = "rrrrrrrrryrrrrrrrrryrrrr";
        }
    }
    else if (currentInterval == phase3_8)
    {
        if (LastDetectedTime[WC_4] < passageTime)
        {
            offSet = ceil(passageTime - LastDetectedTime[WC_4]);
            return;
        }
        else
        {
            nextGreenInterval = phase4_8;
            nextInterval = "rrrrrrrrrrrrrrrgGgGyrrGr";
        }
    }
    else if (currentInterval == phase4_7)
    {
        if (LastDetectedTime[EC_4] < passageTime)
        {
            offSet = ceil(passageTime - LastDetectedTime[EC_4]);
            return;
        }
        else
        {
            nextGreenInterval = phase4_8;
            nextInterval = "rrrrrgGgGyrrrrrrrrrrGrrr";
        }
    }
    else if (currentInterval == phase4_8)
    {
        if (LastDetectedTime[WC_2] < passageTime || LastDetectedTime[WC_3] < passageTime ||
                LastDetectedTime[EC_2] < passageTime || LastDetectedTime[EC_3] < passageTime)
        {
            double smallest1 = ((LastDetectedTime[WC_2] < LastDetectedTime[EC_2]) ? LastDetectedTime[WC_2] : LastDetectedTime[EC_2]);
            double smallest2 = ((LastDetectedTime[WC_3] < LastDetectedTime[EC_3]) ? LastDetectedTime[WC_3] : LastDetectedTime[EC_3]);
            double smallest = ((smallest1 < smallest2) ? smallest1 : smallest2);
            offSet = ceil(passageTime - smallest);
            return;
        }
        else
        {
            nextGreenInterval = phase1_5;
            nextInterval = "rrrrryyyyrrrrrryyyyryryr";
        }
    }

    currentInterval = "yellow";
    TraCI->commandSetTLState("C", nextInterval);
    intervalElapseTime = 0.0;
    offSet =  yellowTime;
}

}

/****************************************************************************/
/// @file    TL_VANET.cc
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

#include <07_TL_VANET.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightVANET);


TrafficLightVANET::~TrafficLightVANET()
{

}


void TrafficLightVANET::initialize(int stage)
{
    TrafficLightWebster::initialize(stage);

    if(TLControlMode != 5)
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

        DetectedTime.assign(lmap.size(), 0.0);
        DetectEvt = new cMessage("DetectEvt", 1);
        scheduleAt(simTime().dbl() + detectFreq, DetectEvt);
    }
}


void TrafficLightVANET::finish()
{
    TrafficLightWebster::finish();

}


void TrafficLightVANET::handleMessage(cMessage *msg)
{
    TrafficLightWebster::handleMessage(msg);

    if(TLControlMode != 5)
        return;

    if (msg == ChangeEvt)
    {
        chooseNextInterval();

        // Schedule next light change event:
        scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);
    }
    else if (msg == DetectEvt)
    {
        list<string> VehList = TraCI->vehicleGetIDList();
        for(list<string>::iterator V = VehList.begin(); V != VehList.end(); V++)
        {
            Coord pos = TraCI->vehicleGetPosition(*V);

            // If within radius of traffic light:
            if ((pos.x > 65 && pos.x < 69) || (pos.x > 131 && pos.x < 135) ||
                    (pos.y > 65 && pos.y < 69) || (pos.y > 131 && pos.y < 135))
            {
                // If in lane of interest (heading towards TL):
                string lane = TraCI->vehicleGetLaneID(*V);
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


void TrafficLightVANET::executeFirstTimeStep()
{
    // call parent
    TrafficLightWebster::executeFirstTimeStep();

    if(TLControlMode != 5)
        return;

    cout << "VANET traffic signal control ..." << endl << endl;

    for (list<string>::iterator TL = TLList.begin(); TL != TLList.end(); TL++)
    {
        TraCI->TLSetProgram(*TL, "adaptive-time");
        TraCI->TLSetState(*TL, phase1_5);
    }

    char buff[300];
    sprintf(buff, "Sim time: %4.2f | Interval finish time: %4.2f | Current interval: %s", simTime().dbl(), simTime().dbl() + intervalOffSet, currentInterval.c_str() );
    cout << buff << endl;

}


void TrafficLightVANET::executeEachTimeStep(bool simulationDone)
{
    // call parent
    TrafficLightWebster::executeEachTimeStep(simulationDone);
}


void TrafficLightVANET::chooseNextInterval()
{
    intervalElapseTime += intervalOffSet;

    if (currentInterval == "yellow")
    {
        currentInterval = "red";

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


void TrafficLightVANET::chooseNextGreenInterval()
{
    // get adjusted VANET detector times
    vector<double> LastDetectedTime;

    for (vector<double>::iterator it = DetectedTime.begin(); it != DetectedTime.end(); it++)
        LastDetectedTime.push_back(simTime().dbl() - (*it));

    // Do proper transition:
    if (currentInterval == phase1_5)
    {
        if (LastDetectedTime[NC_4] < passageTime && LastDetectedTime[SC_4] < passageTime)
        {
            double smallest = ((LastDetectedTime[NC_4] < LastDetectedTime[SC_4]) ? LastDetectedTime[NC_4] : LastDetectedTime[SC_4]);
            intervalOffSet = ceil(passageTime - smallest);

            double extendTime = intervalElapseTime + intervalOffSet;
            // Never extend past maxGreenTime:
            if (extendTime > maxGreenTime)
                intervalOffSet = intervalOffSet - (extendTime - maxGreenTime);

            cout << "Extending green time by: " << intervalOffSet << "s" << endl;
        }
        else if (LastDetectedTime[NC_4] < passageTime)
        {
            nextGreenInterval = phase2_5;
            string nextInterval = "rrrrGrrrrrrrrryrrrrrrrrr";

            currentInterval = "yellow";
            TraCI->TLSetState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
        else if (LastDetectedTime[SC_4] < passageTime)
        {
            nextGreenInterval = phase1_6;
            string nextInterval = "rrrryrrrrrrrrrGrrrrrrrrr";

            currentInterval = "yellow";
            TraCI->TLSetState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
        else
        {
            nextGreenInterval = phase2_6;
            string nextInterval = "rrrryrrrrrrrrryrrrrrrrrr";

            currentInterval = "yellow";
            TraCI->TLSetState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
    }
    else if (currentInterval == phase2_5)
    {
        if (LastDetectedTime[NC_4] < passageTime)
        {
            intervalOffSet = ceil(passageTime - LastDetectedTime[NC_4]);

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
            TraCI->TLSetState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
    }
    else if (currentInterval == phase1_6)
    {
        if (LastDetectedTime[SC_4] < passageTime)
        {
            intervalOffSet = ceil(passageTime - LastDetectedTime[SC_4]);

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
            TraCI->TLSetState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
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
            TraCI->TLSetState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
    }
    else if (currentInterval == phase3_7)
    {
        if (LastDetectedTime[WC_4] < passageTime && LastDetectedTime[EC_4] < passageTime)
        {
            double smallest = ((LastDetectedTime[WC_4] < LastDetectedTime[EC_4]) ? LastDetectedTime[WC_4] : LastDetectedTime[EC_4]);
            intervalOffSet = ceil(passageTime - smallest);

            double extendTime = intervalElapseTime + intervalOffSet;
            // Never extend past maxGreenTime:
            if (extendTime > maxGreenTime)
                intervalOffSet = intervalOffSet - (extendTime - maxGreenTime);

            cout << "Extending green time by: " << intervalOffSet << "s" << endl;
        }
        else if (LastDetectedTime[WC_4] < passageTime)
        {
            nextGreenInterval = phase3_8;
            string nextInterval = "rrrrrrrrryrrrrrrrrrGrrrr";

            currentInterval = "yellow";
            TraCI->TLSetState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
        else if (LastDetectedTime[EC_4] < passageTime)
        {
            nextGreenInterval = phase4_7;
            string nextInterval = "rrrrrrrrrGrrrrrrrrryrrrr";

            currentInterval = "yellow";
            TraCI->TLSetState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
        else
        {
            nextGreenInterval = phase4_8;
            string nextInterval = "rrrrrrrrryrrrrrrrrryrrrr";

            currentInterval = "yellow";
            TraCI->TLSetState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
    }
    else if (currentInterval == phase3_8)
    {
        if (LastDetectedTime[WC_4] < passageTime)
        {
            intervalOffSet = ceil(passageTime - LastDetectedTime[WC_4]);

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
            TraCI->TLSetState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
    }
    else if (currentInterval == phase4_7)
    {
        if (LastDetectedTime[EC_4] < passageTime)
        {
            intervalOffSet = ceil(passageTime - LastDetectedTime[EC_4]);

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
            TraCI->TLSetState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
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
            TraCI->TLSetState("C", nextInterval);

            intervalElapseTime = 0.0;
            intervalOffSet =  yellowTime;
        }
    }
}

}

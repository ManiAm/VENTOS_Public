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
        updateInterval = TraCI->par("updateInterval").doubleValue();
        TLControlMode = par("TLControlMode").longValue();

        minGreenTime = par("minGreenTime").doubleValue();
        maxGreenTime = par("maxGreenTime").doubleValue();
        yellowTime = par("yellowTime").doubleValue();
        redTime = par("redTime").doubleValue();
        passageTime = par("passageTime").doubleValue();
        detectorPos = par("detectorPos").doubleValue();
    }
}


void TrafficLightControl::finish()
{

}


void TrafficLightControl::handleMessage(cMessage *msg)
{

}


void TrafficLightControl::executeFirstTimeStep()
{
    TLList = TraCI->commandGetTLIDList();
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
        nextTime = minGreenTime;
    }
    else
    {
        error("Invalid TLControlMode!");
    }
}


void TrafficLightControl::executeEachTimeStep()
{
    if (TLControlMode == 1)
        return;

    intervalElapseTime += updateInterval;

    char buff[300];
    sprintf(buff, "sim time: %4.2f | Interval finish time: %4.2f | Interval elapsed time: %4.2f | Current interval: %s", simTime().dbl() , nextTime, intervalElapseTime, currentInterval.c_str() );
    cout << buff << endl;

    if (TLControlMode == 2)
    {
        if (simTime() == nextTime || intervalElapseTime >= maxGreenTime)
        {
            string curInterval = currentInterval;

            cout << endl << "Going into doAdaptiveTimeControl()" << endl;
            doAdaptiveTimeControl();

            if(curInterval == currentInterval)
                cout << "Extended green time by: " << passageTime << "s" << endl;

            cout << endl;
        }
    }
    else if (TLControlMode == 3)
    {
        if (simTime() == nextTime || intervalElapseTime >= maxGreenTime)
        {
            string curInterval = currentInterval;

            cout << endl << "Going into doVANETControl()" << endl;
            doAdaptiveTimeControl();

            if(curInterval == currentInterval)
                cout << "Extended green time by: " << passageTime << "s" << endl;

            cout << endl;
        }
    }
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
        nextTime = simTime().dbl() + redTime;

        return;
    }
    else if (currentInterval == "red")
    {
        currentInterval = nextGreenInterval;
        TraCI->commandSetTLState("C", nextGreenInterval);
        intervalElapseTime = 0.0;
        nextTime = simTime().dbl() + minGreenTime;

        return;
    }

    // if we reach here, it means that we are in green interval

    // First get loop detector information
    vector<double> LastActuatedTime;

    // If maxGreenTime met, don't care for actuator values
    if (intervalElapseTime >= maxGreenTime)
        for (list<string>::iterator LD = LDList.begin(); LD != LDList.end(); LD++)
            LastActuatedTime.push_back(passageTime);
    else
        for (list<string>::iterator LD = LDList.begin(); LD != LDList.end(); LD++)
            LastActuatedTime.push_back(TraCI->commandGetLoopDetectorLastTime(*LD));

    // do proper transition:
    if (currentInterval == phase1_5)
    {
        if (LastActuatedTime[NC_4] < passageTime && LastActuatedTime[SC_4] < passageTime)
        {
            nextTime = simTime().dbl() + passageTime;
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
            nextTime = simTime().dbl() + passageTime;
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
            nextTime = simTime().dbl() + passageTime;
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
            nextTime = simTime().dbl() + passageTime;
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
            nextTime = simTime().dbl() + passageTime;
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
            nextTime = simTime().dbl() + passageTime;
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
            nextTime = simTime().dbl() + passageTime;
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
            nextTime = simTime().dbl() + passageTime;
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
    nextTime = simTime().dbl() + yellowTime;
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
        nextTime = simTime().dbl() + redTime;

        return;
    }
    else if (currentInterval == "red")
    {
        currentInterval = nextGreenInterval;
        TraCI->commandSetTLState("C", nextGreenInterval);
        intervalElapseTime = 0.0;
        nextTime = simTime().dbl() + minGreenTime;

        return;
    }

    // if we reach here, it means that we are in green interval

    // First get vehicles in range.
    list<string> VehList = TraCI->commandGetVehicleList();
    vector<bool> VehInLane(12);

    // If maxGreenTime not met, check network:
    if (intervalElapseTime < maxGreenTime)
    {
        for(list<string>::iterator V = VehList.begin(); V != VehList.end(); V++)
        {
            // If within 33m of traffic light:
            Coord pos = TraCI->commandGetVehiclePos(*V);
            if (pos.x > 100 - 33 && pos.x < 100 + 33 && pos.y > 100 - 33 && pos.y < 100 + 33)
            {
                // If in lane of interest:
                string lane = TraCI->commandGetVehicleLaneId(*V);
                if (lane == "EC_2" || lane == "EC_3" || lane == "EC_4" ||
                        lane == "NC_2" || lane == "NC_3" || lane == "NC_4" ||
                        lane == "SC_2" || lane == "SC_3" || lane == "SC_4" ||
                        lane == "WC_2" || lane == "WC_3" || lane == "WC_4")
                {
                    cout << "\nLane: " << lane << " " << lmap[lane] << endl;
                    VehInLane[lmap[lane]] = true;
                }
            }
        }
    }

    // Then do proper transition:
    if (currentInterval == phase1_5)
    {
        if (VehInLane[NC_4] && VehInLane[SC_4])
        {
            nextTime = simTime().dbl() + passageTime;
            return;
        }
        else if (VehInLane[NC_4])
        {
            nextGreenInterval = phase2_5;
            nextInterval = "rrrrGrrrrrrrrryrrrrrrrrr";
        }
        else if (VehInLane[SC_4])
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
        if (VehInLane[NC_4])
        {
            nextTime = simTime().dbl() + passageTime;
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
        if (VehInLane[SC_4])
        {
            nextTime = simTime().dbl() + passageTime;
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
        if (VehInLane[NC_2] || VehInLane[NC_3] ||
                VehInLane[SC_2] || VehInLane[SC_3])
        {
            nextTime = simTime().dbl() + passageTime;
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
        if (VehInLane[WC_4] && VehInLane[EC_4])
        {
            nextTime = simTime().dbl() + passageTime;
            return;
        }
        else if (VehInLane[WC_4])
        {
            nextGreenInterval = phase3_8;
            nextInterval = "rrrrrrrrryrrrrrrrrrGrrrr";
        }
        else if (VehInLane[EC_4])
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
        if (VehInLane[WC_4])
        {
            nextTime = simTime().dbl() + passageTime;
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
        if (VehInLane[EC_4])
        {
            nextTime = simTime().dbl() + passageTime;
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
        if (VehInLane[WC_2] || VehInLane[WC_3] ||
                VehInLane[EC_2] || VehInLane[EC_3])
        {
            nextTime = simTime().dbl() + passageTime;
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
    nextTime = simTime().dbl() + yellowTime;
}

}

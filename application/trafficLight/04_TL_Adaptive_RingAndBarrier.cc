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

#include <04_TL_Adaptive_RingAndBarrier.h>
#include <iomanip>

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
        TrafficLightAdaptive::chooseNextInterval();

        // Schedule next light change event:
        scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);
    }
}


void TrafficLightAdaptive::executeFirstTimeStep()
{
    // call parent
    TrafficLightFixed::executeFirstTimeStep();

    if(TLControlMode != 2)
        return;

    std::cout << "Adaptive-time traffic signal control ..."  << endl;

    // passageTime is not set by the user
    if(passageTime == -1)
    {
        // (*LD).first is 'lane id' and (*LD).second is detector id
        for (std::map<std::string, std::string>::iterator LD = LD_actuated.begin(); LD != LD_actuated.end(); LD++)
        {
            // get the max speed on this lane
            double maxV = TraCI->laneGetMaxSpeed((*LD).first);
            // get position of the loop detector from end of lane
            double LDPos = TraCI->laneGetLength((*LD).first) - TraCI->LDGetPosition((*LD).second);
            // calculate passageTime for this lane
            double pass = std::abs(LDPos) / maxV;
            // check if not greater than Gmin
            if(pass > minGreenTime)
            {
                std::cout << "WARNING: loop detector (" << (*LD).second << ") is far away! Passage time is greater than Gmin." << endl;
                pass = minGreenTime;
            }
            // set it for each lane
            passageTimePerLane[(*LD).first] = pass;
        }
    }
    // passage time is set manually be the user
    else if(passageTime >=0 && passageTime <= minGreenTime)
    {
        // (*LD).first is 'lane id' and (*LD).second is detector id
        for (std::map<std::string, std::string>::iterator LD = LD_actuated.begin(); LD != LD_actuated.end(); LD++)
            passageTimePerLane[(*LD).first] = passageTime;
    }
    else
        error("passageTime value is not set correctly!");

    for (std::list<std::string>::iterator TL = TLList.begin(); TL != TLList.end(); TL++)
    {
        TraCI->TLSetProgram(*TL, "adaptive-time");
        TraCI->TLSetState(*TL, phase1_5);
    }

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << endl << buff << endl << endl;
}


void TrafficLightAdaptive::executeEachTimeStep(bool simulationDone)
{
    // call parent
    TrafficLightFixed::executeEachTimeStep(simulationDone);

    // update passage time if necessary
    if(passageTime == -1)
    {
        // (*LD).first is 'lane id' and (*LD).second is detector id
        for (std::map<std::string, std::string>::iterator LD = LD_actuated.begin(); LD != LD_actuated.end(); LD++)
        {
            double approachSpeed = TraCI->LDGetLastStepMeanVehicleSpeed((*LD).second);
            // update passage time for this lane
            if(approachSpeed > 0)
            {
                // get position of the loop detector from end of lane
                double LDPos = TraCI->laneGetLength((*LD).first) - TraCI->LDGetPosition((*LD).second);
                // calculate passageTime for this lane
                double pass = std::abs(LDPos) / approachSpeed;
                // check if not greater than Gmin
                if(pass > minGreenTime)
                    pass = minGreenTime;

                // update passage time value in passageTimePerLane
                std::map<std::string,double>::iterator location = passageTimePerLane.find((*LD).first);
                location->second = pass;
            }
        }
    }
}


void TrafficLightAdaptive::chooseNextInterval()
{
    intervalElapseTime += intervalOffSet;

    if (currentInterval == "yellow")
    {
        currentInterval = "red";

        // change all 'y' to 'r'
        std::string str = TraCI->TLGetState("C");
        std::string nextInterval = "";
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
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl << endl;
}


void TrafficLightAdaptive::chooseNextGreenInterval()
{
    // print for debugging
    std::cout << "SimTime: " << std::setprecision(2) << std::fixed << simTime().dbl() << " | Passage time value per lane: ";
    for (std::map<std::string,double>::iterator LD = passageTimePerLane.begin(); LD != passageTimePerLane.end(); LD++)
        std::cout << (*LD).first << " (" << (*LD).second << ") | ";
    std::cout << endl;

    // get loop detector information
    std::map<std::string,double> LastActuatedTime;

    std::cout << "SimTime: " << std::setprecision(2) << std::fixed << simTime().dbl() << " | Actuated LDs (lane, elapsed time): ";

    // (*LD).first is 'lane id' and (*LD).second is detector id
    for (std::map<std::string, std::string>::iterator LD = LD_actuated.begin(); LD != LD_actuated.end(); LD++)
    {
        double elapsedT = TraCI->LDGetElapsedTimeLastDetection( (*LD).second);
        LastActuatedTime[(*LD).first] = elapsedT;

        // print for debugging
        if(abs( simTime().dbl() - (elapsedT + updateInterval) ) >= updateInterval)
            std::cout << (*LD).first << " (" << elapsedT << ") | ";
    }

    std::cout << endl;

    bool extend = false;
    std::string nextInterval;

    // Do proper transition:
    if (currentInterval == phase1_5)
    {
        if (intervalElapseTime < maxGreenTime && LastActuatedTime["NC_4"] < passageTimePerLane["NC_4"] && LastActuatedTime["SC_4"] < passageTimePerLane["SC_4"])
        {
            intervalOffSet = std::max(passageTimePerLane["NC_4"]-LastActuatedTime["NC_4"], passageTimePerLane["SC_4"]-LastActuatedTime["SC_4"]);
            extend = true;
        }
        else if (LastActuatedTime["NC_4"] < passageTimePerLane["NC_4"])
        {
            nextGreenInterval = phase2_5;
            nextInterval = "rrrrGrrrrrrrrryrrrrrrrrr";
            extend = false;
        }
        else if (LastActuatedTime["SC_4"] < passageTimePerLane["SC_4"])
        {
            nextGreenInterval = phase1_6;
            nextInterval = "rrrryrrrrrrrrrGrrrrrrrrr";
            extend = false;
        }
        else
        {
            nextGreenInterval = phase2_6;
            nextInterval = "rrrryrrrrrrrrryrrrrrrrrr";
            extend = false;
        }
    }
    else if (currentInterval == phase2_5)
    {
        if (intervalElapseTime < maxGreenTime && LastActuatedTime["NC_4"] < passageTimePerLane["NC_4"])
        {
            intervalOffSet = passageTimePerLane["NC_4"] - LastActuatedTime["NC_4"];
            extend = true;
        }
        else
        {
            nextGreenInterval = phase2_6;
            nextInterval = "gGgGyrrrrrrrrrrrrrrrrrrG";
            extend = false;
        }
    }
    else if (currentInterval == phase1_6)
    {
        if (intervalElapseTime < maxGreenTime && LastActuatedTime["SC_4"] < passageTimePerLane["SC_4"])
        {
            intervalOffSet = passageTimePerLane["SC_4"] - LastActuatedTime["SC_4"];
            extend = true;
        }
        else
        {
            nextGreenInterval = phase2_6;
            nextInterval = "rrrrrrrrrrgGgGyrrrrrrGrr";
            extend = false;
        }
    }
    else if (currentInterval == phase2_6)
    {
        if (intervalElapseTime < maxGreenTime &&
            (LastActuatedTime["NC_2"] < passageTimePerLane["NC_2"] || LastActuatedTime["NC_3"] < passageTimePerLane["NC_3"] ||
             LastActuatedTime["SC_2"] < passageTimePerLane["SC_2"] || LastActuatedTime["SC_3"] < passageTimePerLane["SC_3"]))
        {
            double biggest1 = std::max(passageTimePerLane["NC_2"]-LastActuatedTime["NC_2"], passageTimePerLane["SC_2"]-LastActuatedTime["SC_2"]);
            double biggest2 = std::max(passageTimePerLane["NC_3"]-LastActuatedTime["NC_3"], passageTimePerLane["SC_3"]-LastActuatedTime["SC_3"]);
            intervalOffSet = std::max(biggest1, biggest2);
            extend = true;
        }
        else
        {
            nextGreenInterval = phase3_7;
            nextInterval = "yyyyrrrrrryyyyrrrrrrryry";
            extend = false;
        }
    }
    else if (currentInterval == phase3_7)
    {
        if (intervalElapseTime < maxGreenTime && LastActuatedTime["WC_4"] < passageTimePerLane["WC_4"] && LastActuatedTime["EC_4"] < passageTimePerLane["EC_4"])
        {
            intervalOffSet = std::max(passageTimePerLane["WC_4"]-LastActuatedTime["WC_4"], passageTimePerLane["EC_4"]-LastActuatedTime["EC_4"]);
            extend = true;
        }
        else if (LastActuatedTime["WC_4"] < passageTimePerLane["WC_4"])
        {
            nextGreenInterval = phase3_8;
            nextInterval = "rrrrrrrrryrrrrrrrrrGrrrr";
            extend = false;
        }
        else if (LastActuatedTime["EC_4"] < passageTimePerLane["EC_4"])
        {
            nextGreenInterval = phase4_7;
            nextInterval = "rrrrrrrrrGrrrrrrrrryrrrr";
            extend = false;
        }
        else
        {
            nextGreenInterval = phase4_8;
            nextInterval = "rrrrrrrrryrrrrrrrrryrrrr";
            extend = false;
        }
    }
    else if (currentInterval == phase3_8)
    {
        if (intervalElapseTime < maxGreenTime && LastActuatedTime["WC_4"] < passageTimePerLane["WC_4"])
        {
            intervalOffSet = passageTimePerLane["WC_4"] - LastActuatedTime["WC_4"];
            extend = true;
        }
        else
        {
            nextGreenInterval = phase4_8;
            nextInterval = "rrrrrrrrrrrrrrrgGgGyrrGr";
            extend = false;
        }
    }
    else if (currentInterval == phase4_7)
    {
        if (intervalElapseTime < maxGreenTime && LastActuatedTime["EC_4"] < passageTimePerLane["EC_4"])
        {
            intervalOffSet = passageTimePerLane["EC_4"] - LastActuatedTime["EC_4"];
            extend = true;
        }
        else
        {
            nextGreenInterval = phase4_8;
            nextInterval = "rrrrrgGgGyrrrrrrrrrrGrrr";
            extend = false;
        }
    }
    else if (currentInterval == phase4_8)
    {
        if (intervalElapseTime < maxGreenTime &&
            (LastActuatedTime["WC_2"] < passageTimePerLane["WC_2"] || LastActuatedTime["WC_3"] < passageTimePerLane["WC_3"] ||
            LastActuatedTime["EC_2"] < passageTimePerLane["EC_2"] || LastActuatedTime["EC_3"] < passageTimePerLane["EC_3"]))
        {
            double biggest1 = std::max(passageTimePerLane["WC_2"]-LastActuatedTime["WC_2"], passageTimePerLane["EC_2"]-LastActuatedTime["EC_2"]);
            double biggest2 = std::max(passageTimePerLane["WC_3"]-LastActuatedTime["WC_3"], passageTimePerLane["EC_3"]-LastActuatedTime["EC_3"]);
            intervalOffSet = std::max(biggest1, biggest2);
            extend = true;
        }
        else
        {
            nextGreenInterval = phase1_5;
            nextInterval = "rrrrryyyyrrrrrryyyyryryr";
            extend = false;
        }
    }

    // the current green interval should be extended
    if(extend)
    {
        // give a lower bound
        intervalOffSet = std::max(updateInterval, intervalOffSet);

        // interval duration after this offset
        double newIntervalTime = intervalElapseTime + intervalOffSet;

        // never extend past maxGreenTime
        if (newIntervalTime > maxGreenTime)
            intervalOffSet = intervalOffSet - (newIntervalTime - maxGreenTime);

        std::cout << ">>> Extending green time by " << intervalOffSet << "s" << endl;
    }
    // we should terminate the current green interval
    else
    {
        currentInterval = "yellow";
        TraCI->TLSetState("C", nextInterval);

        intervalElapseTime = 0.0;
        intervalOffSet =  yellowTime;
    }
}


}

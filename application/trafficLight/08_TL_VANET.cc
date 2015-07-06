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

#include <08_TL_VANET.h>
#include <iomanip>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightVANET);


TrafficLightVANET::~TrafficLightVANET()
{

}


void TrafficLightVANET::initialize(int stage)
{
    TrafficLightAdaptiveQueue::initialize(stage);

    if(TLControlMode != TL_VANET)
        return;

    if(stage == 0)
    {
        // NED variables
        greenExtension = par("greenExtension").boolValue();

        ChangeEvt = new cMessage("ChangeEvt", 1);
    }
}


void TrafficLightVANET::finish()
{
    TrafficLightAdaptiveQueue::finish();
}


void TrafficLightVANET::handleMessage(cMessage *msg)
{
    TrafficLightAdaptiveQueue::handleMessage(msg);

    if(TLControlMode != TL_VANET)
        return;

    if (msg == ChangeEvt)
    {
        chooseNextInterval();

        if(intervalOffSet <= 0)
            error("intervalOffSet is <= 0");

        // Schedule next light change event:
        scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);
    }
}


void TrafficLightVANET::executeFirstTimeStep()
{
    // call parent
    TrafficLightAdaptiveQueue::executeFirstTimeStep();

    if(TLControlMode != TL_VANET)
        return;

    std::cout << endl << "VANET traffic signal control ..." << endl;

    // set initial values
    currentInterval = phase1_5;
    intervalElapseTime = 0;
    intervalOffSet = minGreenTime;

    scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);

    for (std::list<std::string>::iterator TL = TLList.begin(); TL != TLList.end(); ++TL)
    {
        TraCI->TLSetProgram(*TL, "adaptive-time");
        TraCI->TLSetState(*TL, currentInterval);

        firstGreen[*TL] = currentInterval;

        // initialize TL status
        updateTLstate(*TL, "init", currentInterval);
    }

    // get a pointer to the RSU module that controls this TL
    findRSU("C");

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << endl << buff << endl << endl;
}


void TrafficLightVANET::executeEachTimeStep(bool simulationDone)
{
    // call parent
    TrafficLightAdaptiveQueue::executeEachTimeStep(simulationDone);

    if(TLControlMode != TL_VANET)
        return;

    intervalElapseTime += updateInterval;
}


void TrafficLightVANET::findRSU(std::string TLid)
{
    // get a pointer to the RSU module that controls this intersection
    cModule *module = simulation.getSystemModule()->getSubmodule("RSU", 0);
    if(module == NULL)
        error("No RSU module was found in the network!");

    // how many RSUs are in the network?
    int RSUcount = module->getVectorSize();

    // iterate over RSUs
    bool found = false;
    for(int i = 0; i < RSUcount; ++i)
    {
        module = simulation.getSystemModule()->getSubmodule("RSU", i);
        cModule *appl =  module->getSubmodule("appl");
        std::string myTLid = appl->par("myTLid").stringValue();

        // we found our RSU
        if(myTLid == TLid)
        {
            RSU = static_cast<ApplRSUTLVANET *>(appl);
            if(RSU == NULL)
                error("Can not get a reference to our RSU!");

            found = true;
            break;
        }
    }

    if(!found)
        error("TL %s does not have any RSU!", TLid.c_str());
}


void TrafficLightVANET::chooseNextInterval()
{
    if (currentInterval == "yellow")
    {
        currentInterval = "red";

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

        // update TL status for this phase
        updateTLstate("C", "red");

        char buff[300];
        sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
        std::cout << buff << endl << endl;
    }
    else if (currentInterval == "red")
    {
        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState("C", nextGreenInterval);
        intervalElapseTime = 0.0;
        intervalOffSet = minGreenTime;

        // update TL status for this phase
        if(nextGreenInterval == firstGreen["C"])
            updateTLstate("C", "phaseEnd", nextGreenInterval, true);
        else
            updateTLstate("C", "phaseEnd", nextGreenInterval);

        char buff[300];
        sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
        std::cout << buff << endl << endl;
    }
    else
        chooseNextGreenInterval();
}


void TrafficLightVANET::chooseNextGreenInterval()
{
    // print for debugging
    std::cout << "SimTime: " << std::setprecision(2) << std::fixed << simTime().dbl() << " | Passage time value per lane: ";
    for (std::map<std::string,double>::iterator LD = RSU->passageTimePerLane.begin(); LD != RSU->passageTimePerLane.end(); ++LD)
        std::cout << (*LD).first << " (" << (*LD).second << ") | ";
    std::cout << endl;

    // get adjusted VANET detector times
    std::map<std::string,double> LastDetectedTime;

    for (std::map<std::string, double>::iterator it = RSU->detectedTime.begin(); it != RSU->detectedTime.end(); ++it)
    {
        std::string lane = (*it).first;
        double time = (*it).second;
        LastDetectedTime[lane] = simTime().dbl() - time;
    }

    bool extend = false;
    std::string nextInterval;

    // Do proper transition:
    if (currentInterval == phase1_5)
    {
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                LastDetectedTime["NC_4"] < RSU->passageTimePerLane["NC_4"] &&
                LastDetectedTime["SC_4"] < RSU->passageTimePerLane["SC_4"])
        {
            intervalOffSet = std::max(RSU->passageTimePerLane["NC_4"]-LastDetectedTime["NC_4"], RSU->passageTimePerLane["SC_4"]-LastDetectedTime["SC_4"]);
            extend = true;
        }
        else if (LastDetectedTime["NC_4"] < RSU->passageTimePerLane["NC_4"])
        {
            nextGreenInterval = phase2_5;
            nextInterval = "rrrrGrrrrrrrrryrrrrrrrrr";
            extend = false;
        }
        else if (LastDetectedTime["SC_4"] < RSU->passageTimePerLane["SC_4"])
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
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                LastDetectedTime["NC_4"] < RSU->passageTimePerLane["NC_4"])
        {
            intervalOffSet = RSU->passageTimePerLane["NC_4"] - LastDetectedTime["NC_4"];
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
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                LastDetectedTime["SC_4"] < RSU->passageTimePerLane["SC_4"])
        {
            intervalOffSet = RSU->passageTimePerLane["SC_4"] - LastDetectedTime["SC_4"];
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
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                (LastDetectedTime["NC_2"] < RSU->passageTimePerLane["NC_2"] ||
                        LastDetectedTime["NC_3"] < RSU->passageTimePerLane["NC_3"] ||
                        LastDetectedTime["SC_2"] < RSU->passageTimePerLane["SC_2"] ||
                        LastDetectedTime["SC_3"] < RSU->passageTimePerLane["SC_3"]))
        {
            double biggest1 = std::max(RSU->passageTimePerLane["NC_2"]-LastDetectedTime["NC_2"], RSU->passageTimePerLane["SC_2"]-LastDetectedTime["SC_2"]);
            double biggest2 = std::max(RSU->passageTimePerLane["NC_3"]-LastDetectedTime["NC_3"], RSU->passageTimePerLane["SC_3"]-LastDetectedTime["SC_3"]);
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
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                LastDetectedTime["WC_4"] < RSU->passageTimePerLane["WC_4"] &&
                LastDetectedTime["EC_4"] < RSU->passageTimePerLane["EC_4"])
        {
            intervalOffSet = std::max(RSU->passageTimePerLane["WC_4"]-LastDetectedTime["WC_4"], RSU->passageTimePerLane["EC_4"]-LastDetectedTime["EC_4"]);
            extend = true;
        }
        else if (LastDetectedTime["WC_4"] < RSU->passageTimePerLane["WC_4"])
        {
            nextGreenInterval = phase3_8;
            nextInterval = "rrrrrrrrryrrrrrrrrrGrrrr";
            extend = false;
        }
        else if (LastDetectedTime["EC_4"] < RSU->passageTimePerLane["EC_4"])
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
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                LastDetectedTime["WC_4"] < RSU->passageTimePerLane["WC_4"])
        {
            intervalOffSet = RSU->passageTimePerLane["WC_4"] - LastDetectedTime["WC_4"];
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
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                LastDetectedTime["EC_4"] < RSU->passageTimePerLane["EC_4"])
        {
            intervalOffSet = RSU->passageTimePerLane["EC_4"] - LastDetectedTime["EC_4"];
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
        if (greenExtension && intervalElapseTime < maxGreenTime &&
                (LastDetectedTime["WC_2"] < RSU->passageTimePerLane["WC_2"] ||
                        LastDetectedTime["WC_3"] < RSU->passageTimePerLane["WC_3"] ||
                        LastDetectedTime["EC_2"] < RSU->passageTimePerLane["EC_2"] ||
                        LastDetectedTime["EC_3"] < RSU->passageTimePerLane["EC_3"]))
        {
            double biggest1 = std::max(RSU->passageTimePerLane["WC_2"]-LastDetectedTime["WC_2"], RSU->passageTimePerLane["EC_2"]-LastDetectedTime["EC_2"]);
            double biggest2 = std::max(RSU->passageTimePerLane["WC_3"]-LastDetectedTime["WC_3"], RSU->passageTimePerLane["EC_3"]-LastDetectedTime["EC_3"]);
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

        // offset can not be too small
        if(intervalOffSet < updateInterval)
        {
            intervalOffSet = 0.0001;
            intervalElapseTime = maxGreenTime;
            std::cout << ">>> Green extension offset is too small. Terminating the current phase ..." << endl << endl;
        }
        else
            std::cout << ">>> Extending green for both movements by " << intervalOffSet << "s" << endl << endl;
    }
    // we should terminate the current green interval
    else
    {
        currentInterval = "yellow";
        TraCI->TLSetState("C", nextInterval);

        intervalElapseTime = 0.0;
        intervalOffSet =  yellowTime;

        // update TL status for this phase
        updateTLstate("C", "yellow");

        char buff[300];
        sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
        std::cout << buff << endl << endl;
    }
}

}

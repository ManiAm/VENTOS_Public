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

    if(TLControlMode != TL_VANET)
        return;

    if(stage == 0)
    {
        // set initial values
        intervalOffSet = minGreenTime;
        intervalElapseTime = 0;
        currentInterval = phase1_5;

        ChangeEvt = new cMessage("ChangeEvt", 1);
        scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);
    }
}


void TrafficLightVANET::finish()
{
    TrafficLightWebster::finish();
}


void TrafficLightVANET::handleMessage(cMessage *msg)
{
    TrafficLightWebster::handleMessage(msg);

    if(TLControlMode != TL_VANET)
        return;

    if (msg == ChangeEvt)
    {
        chooseNextInterval();

        // Schedule next light change event:
        scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);
    }
}


void TrafficLightVANET::executeFirstTimeStep()
{
    // call parent
    TrafficLightWebster::executeFirstTimeStep();

    if(TLControlMode != TL_VANET)
        return;

    std::cout << endl << "VANET traffic signal control ..." << endl << endl;

    for (std::list<std::string>::iterator TL = TLList.begin(); TL != TLList.end(); ++TL)
    {
        TraCI->TLSetProgram(*TL, "adaptive-time");
        TraCI->TLSetState(*TL, currentInterval);
    }

    // get a pointer to the RSU module that controls this intersection
    cModule *module = simulation.getSystemModule()->getSubmodule("RSU", 0)->getSubmodule("appl");
    if(module == NULL)
        error("No RSU module found!");
    RSU = static_cast<ApplRSUTLVANET *>(module);
    if(RSU == NULL)
        error("No pointer to the RSU module!");

    char buff[300];
    sprintf(buff, "Sim time: %4.2f | Interval finish time: %4.2f | Current interval: %s", simTime().dbl(), simTime().dbl() + intervalOffSet, currentInterval.c_str() );
    std::cout << buff << endl;
}


void TrafficLightVANET::executeEachTimeStep(bool simulationDone)
{
    // call parent
    TrafficLightWebster::executeEachTimeStep(simulationDone);

    if(TLControlMode != TL_VANET)
        return;

    intervalElapseTime += updateInterval;
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

        char buff[300];
        sprintf(buff, "Sim time: %4.2f | Interval finish time: %4.2f | Current interval: %s", simTime().dbl(), simTime().dbl() + intervalOffSet, currentInterval.c_str() );
        std::cout << buff << endl;
    }
    else if (currentInterval == "red")
    {
        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState("C", nextGreenInterval);
        intervalElapseTime = 0.0;
        intervalOffSet = minGreenTime;

        char buff[300];
        sprintf(buff, "Sim time: %4.2f | Interval finish time: %4.2f | Current interval: %s", simTime().dbl(), simTime().dbl() + intervalOffSet, currentInterval.c_str() );
        std::cout << buff << endl;
    }
    else
        chooseNextGreenInterval();
}


void TrafficLightVANET::chooseNextGreenInterval()
{
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
        if (greenExtension && intervalElapseTime < maxGreenTime && LastDetectedTime["NC_4"] < passageTime && LastDetectedTime["SC_4"] < passageTime)
        {
            intervalOffSet = std::max(passageTime-LastDetectedTime["NC_4"], passageTime-LastDetectedTime["SC_4"]);
            extend = true;
        }
        else if (LastDetectedTime["NC_4"] < passageTime)
        {
            nextGreenInterval = phase2_5;
            nextInterval = "rrrrGrrrrrrrrryrrrrrrrrr";
            extend = false;
        }
        else if (LastDetectedTime["SC_4"] < passageTime)
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
        if (greenExtension && intervalElapseTime < maxGreenTime && LastDetectedTime["NC_4"] < passageTime)
        {
            intervalOffSet = passageTime - LastDetectedTime["NC_4"];
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
        if (greenExtension && intervalElapseTime < maxGreenTime && LastDetectedTime["SC_4"] < passageTime)
        {
            intervalOffSet = passageTime - LastDetectedTime["SC_4"];
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
             (LastDetectedTime["NC_2"] < passageTime || LastDetectedTime["NC_3"] < passageTime ||
              LastDetectedTime["SC_2"] < passageTime || LastDetectedTime["SC_3"] < passageTime))
        {
            double biggest1 = std::max(passageTime-LastDetectedTime["NC_2"], passageTime-LastDetectedTime["SC_2"]);
            double biggest2 = std::max(passageTime-LastDetectedTime["NC_3"], passageTime-LastDetectedTime["SC_3"]);
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
        if (greenExtension && intervalElapseTime < maxGreenTime && LastDetectedTime["WC_4"] < passageTime && LastDetectedTime["EC_4"] < passageTime)
        {
            intervalOffSet = std::max(passageTime-LastDetectedTime["WC_4"], passageTime-LastDetectedTime["EC_4"]);
            extend = true;
        }
        else if (LastDetectedTime["WC_4"] < passageTime)
        {
            nextGreenInterval = phase3_8;
            nextInterval = "rrrrrrrrryrrrrrrrrrGrrrr";
            extend = false;
        }
        else if (LastDetectedTime["EC_4"] < passageTime)
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
        if (greenExtension && intervalElapseTime < maxGreenTime && LastDetectedTime["WC_4"] < passageTime)
        {
            intervalOffSet = passageTime - LastDetectedTime["WC_4"];
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
        if (greenExtension && intervalElapseTime < maxGreenTime && LastDetectedTime["EC_4"] < passageTime)
        {
            intervalOffSet = passageTime - LastDetectedTime["EC_4"];
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
            (LastDetectedTime["WC_2"] < passageTime || LastDetectedTime["WC_3"] < passageTime ||
             LastDetectedTime["EC_2"] < passageTime || LastDetectedTime["EC_3"] < passageTime))
        {
            double biggest1 = std::max(passageTime-LastDetectedTime["WC_2"], passageTime-LastDetectedTime["EC_2"]);
            double biggest2 = std::max(passageTime-LastDetectedTime["WC_3"], passageTime-LastDetectedTime["EC_3"]);
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

        char buff[300];
        sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
        std::cout << buff << endl << endl;
    }
}

}

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

#include <04_TL_Adaptive_Webster.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightWebster);


TrafficLightWebster::~TrafficLightWebster()
{

}


void TrafficLightWebster::initialize(int stage)
{
    TrafficLightFixed::initialize(stage);

    minGreenTime = par("minGreenTime").doubleValue();
    maxGreenTime = par("maxGreenTime").doubleValue();
    yellowTime = par("yellowTime").doubleValue();
    redTime = par("redTime").doubleValue();

    if(minGreenTime <= 0)
        error("minGreenTime value is wrong!");
    if(maxGreenTime <= 0 || maxGreenTime < minGreenTime)
        error("maxGreenTime value is wrong!");
    if(yellowTime <= 0)
        error("yellowTime value is wrong!");
    if(redTime <= 0)
        error("redTime value is wrong!");

    if(TLControlMode != TL_Adaptive_Webster)
        return;

    if(stage == 0)
    {
        ChangeEvt = new cMessage("ChangeEvt", 1);

        greenSplit.clear();

        // add phases into greenSplit
        greenSplit[phase1_5] = 0;
        greenSplit[phase2_6] = 0;
        greenSplit[phase3_7] = 0;
        greenSplit[phase4_8] = 0;

        // measure traffic demand
        measureTrafficDemand = true;
    }
}


void TrafficLightWebster::finish()
{
    TrafficLightFixed::finish();
}


void TrafficLightWebster::handleMessage(cMessage *msg)
{
    TrafficLightFixed::handleMessage(msg);

    if(TLControlMode != TL_Adaptive_Webster)
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


void TrafficLightWebster::executeFirstTimeStep()
{
    TrafficLightFixed::executeFirstTimeStep();

    if(TLControlMode != TL_Adaptive_Webster)
        return;

    std::cout << endl << "Dynamic Webster traffic signal control ... " << endl << endl;

    // run Webster at the beginning of the cycle
    calculateGreenSplits();

    // set initial values
    currentInterval = phase1_5;
    intervalElapseTime = 0;
    intervalOffSet = greenSplit[phase1_5];

    scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);

    for (std::list<std::string>::iterator TL = TLList.begin(); TL != TLList.end(); ++TL)
    {
        TraCI->TLSetProgram(*TL, "adaptive-time");
        TraCI->TLSetState(*TL, currentInterval);

        if(collectTLData)
        {
            // initialize phase number in this TL
            phaseTL[*TL] = 1;

            // get all incoming lanes
            std::list<std::string> lan = TraCI->TLGetControlledLanes(*TL);

            // remove duplicate entries
            lan.unique();

            // Initialize status in this TL
            currentStatusTL *entry = new currentStatusTL(currentInterval, simTime().dbl(), -1, -1, -1, lan.size(), -1);
            statusTL.insert( std::make_pair(std::make_pair(*TL,1), *entry) );
        }
    }

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << endl << buff << endl << endl;
}


void TrafficLightWebster::executeEachTimeStep(bool simulationDone)
{
    TrafficLightFixed::executeEachTimeStep(simulationDone);

    if(TLControlMode != TL_Adaptive_Webster)
        return;

    intervalElapseTime += updateInterval;
}


void TrafficLightWebster::chooseNextInterval()
{
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

        if(collectTLData)
        {
            // update TL status for this phase
            std::map<std::pair<std::string,int>, currentStatusTL>::iterator location = statusTL.find( std::make_pair("C",phaseTL["C"]) );
            (location->second).redStart = simTime().dbl();
        }
    }
    else if (currentInterval == "red")
    {
        // run Webster at the beginning of the cycle
        if(nextGreenInterval == phase1_5)
        {
            calculateGreenSplits();
        }

        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState("C", nextGreenInterval);
        intervalElapseTime = 0.0;
        intervalOffSet = greenSplit[nextGreenInterval];

        if(collectTLData)
        {
            // get all incoming lanes
            std::list<std::string> lan = TraCI->TLGetControlledLanes("C");

            // remove duplicate entries
            lan.unique();

            // for each incoming lane
            int totalQueueSize = 0;
            for(std::list<std::string>::iterator it2 = lan.begin(); it2 != lan.end(); ++it2)
            {
                totalQueueSize = totalQueueSize + laneQueueSize[*it2].second;
            }

            // update TL status for this phase
            std::map<std::pair<std::string,int>, currentStatusTL>::iterator location = statusTL.find( std::make_pair("C",phaseTL["C"]) );
            (location->second).phaseEnd = simTime().dbl();
            (location->second).totalQueueSize = totalQueueSize;

            // increase phase number by 1
            std::map<std::string, int>::iterator location2 = phaseTL.find("C");
            location2->second = location2->second + 1;

            // update status for the new phase
            currentStatusTL *entry = new currentStatusTL(nextGreenInterval, simTime().dbl(), -1, -1, -1, lan.size(), -1);
            statusTL.insert( std::make_pair(std::make_pair("C",location2->second), *entry) );
        }
    }
    else
        chooseNextGreenInterval();

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl << endl;
}


void TrafficLightWebster::chooseNextGreenInterval()
{
    std::string nextInterval;

    if(currentInterval == phase1_5)
    {
        nextGreenInterval = phase2_6;
        nextInterval = "grgrygrgrrgrgrygrgrrrrrr";
    }
    else if(currentInterval == phase2_6)
    {
        nextGreenInterval = phase3_7;
        nextInterval = "gygyrgrgrrgygyrgrgrrryry";
    }
    else if(currentInterval == phase3_7)
    {
        nextGreenInterval = phase4_8;
        nextInterval = "grgrrgrgrygrgrrgrgryrrrr";
    }
    else if(currentInterval == phase4_8)
    {
        nextGreenInterval = phase1_5;
        nextInterval = "grgrrgygyrgrgrrgygyryryr";
    }

    currentInterval = "yellow";
    TraCI->TLSetState("C", nextInterval);

    intervalElapseTime = 0.0;
    intervalOffSet =  yellowTime;

    if(collectTLData)
    {
        // update TL status for this phase
        std::map<std::pair<std::string,int>, currentStatusTL>::iterator location = statusTL.find( std::make_pair("C",phaseTL["C"]) );
        (location->second).yellowStart = simTime().dbl();
    }
}


void TrafficLightWebster::calculateGreenSplits()
{
    double demand1_5 = 1500.;
    double demand2_6 = 1400.;
    double demand3_7 = 1300.;
    double demand4_8 = 1200.;

    double saturation1_5 = 8000.;
    double saturation2_6 = 8000.;
    double saturation3_7 = 8000.;
    double saturation4_8 = 8000.;

    double Y1_5 = demand1_5 / saturation1_5;
    double Y2_6 = demand2_6 / saturation2_6;
    double Y3_7 = demand3_7 / saturation3_7;
    double Y4_8 = demand4_8 / saturation4_8;

    double Y = Y1_5 + Y2_6 + Y3_7 + Y4_8;

    // make sure Y < 1
    if(Y >= 1)
        error("total critical-to-capacity ratio (Y) >= 1");

    double L = (yellowTime + redTime) * 4;   // total loss time in cycle
    double cycle = ((1.5*L) + 5) / (1 - Y);  // cycle length

    double effectiveG = cycle - L;   // total effective green time

    // green split for each phase
    greenSplit[phase1_5] = (Y1_5 / Y) * effectiveG;
    greenSplit[phase2_6] = (Y2_6 / Y) * effectiveG;
    greenSplit[phase3_7] = (Y3_7 / Y) * effectiveG;
    greenSplit[phase4_8] = (Y4_8 / Y) * effectiveG;

    std::cout << ">>> Updating green splits for each phase: ";
    for(std::map<std::string, double>::iterator y = greenSplit.begin(); y != greenSplit.end(); y++)
    {
        double split = (*y).second;
        std::cout << split << ", ";
        if(split <= 0)
            error("greenSplit can not be <= 0");

    }
    std::cout << endl;






    //    // debugging
    //    if(LDid == "demand_WC_3")
    //    {
    //        std::map<std::string, std::pair<std::string, boost::circular_buffer<double>>>::iterator loc = laneTD.find(lane);
    //        boost::circular_buffer<double> tmp ( (loc->second).second );
    //        for(boost::circular_buffer<double>::iterator y = tmp.begin(); y != tmp.end(); ++y)
    //        {
    //            std::cout << (*y) << " | ";
    //        }
    //
    //        std::cout << endl;
    //    }

    //            // calculate sum
    //            double sum = 0;
    //            for (boost::circular_buffer<double>::iterator it = MyCircularBufferMerge.begin(); it != MyCircularBufferMerge.end(); it++)
    //                sum = sum + *it;
    //
    //            // calculate average
    //            double avg = sum / MyCircularBufferMerge.size();
}

}

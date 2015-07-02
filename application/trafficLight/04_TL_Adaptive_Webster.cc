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
    std::cout << buff << endl << endl;
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
    // print traffic demand
    std::cout << ">>> Measured traffic demands at the beginning of this cycle: " << endl;
    for(std::map<std::string, std::pair<std::string, boost::circular_buffer<double>>>::iterator y = laneTD.begin(); y != laneTD.end(); ++y)
    {
        std::string lane = (*y).first;
        std::string TLid = (*y).second.first;
        boost::circular_buffer<double> buf = (*y).second.second;

        if(!buf.empty())
        {
            // calculate average TD
            double sum = 0;
            for (boost::circular_buffer<double>::iterator it = buf.begin(); it != buf.end(); ++it)
                sum = sum + *it;
            double aveTD = sum / (double)buf.size();

            std::cout << lane << ": ";
            for(boost::circular_buffer<double>::iterator z = buf.begin(); z != buf.end(); ++z)
                std::cout << (*z) << ", ";

            std::cout << "---> Ave= " << aveTD << endl;
        }
    }
    std::cout << endl;

    // todo: change this later
    // saturation = (3*TD) / ( 1-(35/cycle) )
    // max TD = 1900, max cycle = 120
    double saturation = 2400;  //8047;

    std::string phases[] = {phase1_5, phase2_6, phase3_7, phase4_8};
    std::map<std::string, double> critical;

    double Y = 0;
    for (std::string prog : phases)
    {
        double Y_i = -1;  // critical volume-to-capacity ratio for this movement batch
        // for each link in this batch
        for(unsigned int i = 0; i < prog.size(); ++i)
        {
            // if link i is active
            if(prog[i] == 'g' || prog[i] == 'G')
            {
                // if link i is not a right-turn
                // right turns are all permissive
                bool notRightTurn = std::find(std::begin(rightTurns), std::end(rightTurns), i) == std::end(rightTurns);
                if(notRightTurn)
                {
                    // get all TD measurements for link i so far
                    boost::circular_buffer<double> buffer = linkTD[std::make_pair("C",i)];

                    // calculate average TD for link i
                    double sum = 0;
                    for (boost::circular_buffer<double>::iterator it = buffer.begin(); it != buffer.end(); ++it)
                        sum = sum + *it;
                    double aveTD = buffer.size() == 0 ? 0 : sum / (double)buffer.size();

                    Y_i = std::max(Y_i, aveTD / saturation);
                }
            }
        }

        critical[prog] = Y_i;
        Y = Y + Y_i;
    }

    // print Y_i for each phase
    std::cout << ">>> critical v/c for each phase: ";
    int activePhases = 0;
    for(std::map<std::string, double>::iterator y = critical.begin(); y != critical.end(); ++y)
    {
        double val = (*y).second;
        std::cout << val << ", ";
        if(val != 0) activePhases++;
    }
    std::cout << endl << endl;

    if(Y < 0)
    {
        error("WTH! total critical v/c is negative!");
    }
    // no TD in any directions. Give G_min to each phase
    else if(Y == 0)
    {
        // green split for each phase
        for (std::string prog : phases)
            greenSplit[prog] = minGreenTime;

        std::cout << ">>> Total critical v/c is zero! Set green split for each phase to G_min=" << minGreenTime << endl << endl;
    }
    else if(Y > 0 && Y < 1)
    {
        double startupLoss_i = 1.0;
        double changeInterval_i = yellowTime + redTime;
        double clearanceLoss_i = (1-0.8)*changeInterval_i;
        double totalLoss_i = startupLoss_i + clearanceLoss_i;   // total loss time per phase
        double totalLoss = totalLoss_i * activePhases;  // total loss time per phase

        double cycle = ((1.5*totalLoss) + 5) / (1 - Y);  // cycle length

        std::cout << ">>> Webster Calculation: " << endl;
        std::cout << "total critical v/c=" << Y << ", total loss time=" << totalLoss << ", cycle length=" << cycle << endl;

        // make sure that cycle length is not too big.
        // this happens when Y is too close to 1
        if(cycle > 120)
        {
            std::cout << "WARNING: cycle length > 120. Set it to 120." << endl;
            cycle = 120;
        }

        double effectiveG = cycle - totalLoss;   // total effective green time

        // green split for each phase
        for (std::string prog : phases)
        {
            double GS = (critical[prog] / Y) * effectiveG;
            greenSplit[prog] = (GS == 0) ? minGreenTime : GS;
        }

        std::cout << "Updating green splits for each phase: ";
        for(std::map<std::string, double>::iterator y = greenSplit.begin(); y != greenSplit.end(); y++)
        {
            double split = (*y).second;
            if(split < 0) error("greenSplit can not be <= 0");
            std::cout << split << ", ";
        }
        std::cout << endl << endl;
    }
    else if(Y >= 1)
    {
        error("total critical v/c >= 1. Saturation flow might be low ?!");
    }
}

}

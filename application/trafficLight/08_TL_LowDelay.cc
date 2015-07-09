/****************************************************************************/
/// @file    TL_LowDelay.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @date    Jul 2015
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

#include <08_TL_LowDelay.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightLowDelay);


TrafficLightLowDelay::~TrafficLightLowDelay()
{

}


void TrafficLightLowDelay::initialize(int stage)
{
    TrafficLightActuated::initialize(stage);

    if(TLControlMode != TL_LowDelay)
        return;

    if(stage == 0)
    {
        ChangeEvt = new cMessage("ChangeEvt", 1);
    }
}


void TrafficLightLowDelay::finish()
{
    TrafficLightActuated::finish();
}


void TrafficLightLowDelay::handleMessage(cMessage *msg)
{
    TrafficLightActuated::handleMessage(msg);

    if(TLControlMode != TL_LowDelay)
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


void TrafficLightLowDelay::executeFirstTimeStep()
{
    TrafficLightActuated::executeFirstTimeStep();

    if(TLControlMode != TL_LowDelay)
        return;

    std::cout << endl << "Low delay traffic signal control ... " << endl << endl;

    // set initial values
    currentInterval = phase1_5;
    intervalElapseTime = 0;
    intervalOffSet = minGreenTime;

    scheduleAt(simTime().dbl() + intervalOffSet, ChangeEvt);

    // get all non-conflicting movements in allMovements vector
    TrafficLightAllowedMoves::getMovements("C");

    for (std::list<std::string>::iterator TL = TLList.begin(); TL != TLList.end(); ++TL)
    {
        TraCI->TLSetProgram(*TL, "adaptive-time");
        TraCI->TLSetState(*TL, currentInterval);

        firstGreen[*TL] = currentInterval;

        // initialize TL status
        updateTLstate(*TL, "init", currentInterval);
    }

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl << endl;
}


void TrafficLightLowDelay::executeEachTimeStep(bool simulationDone)
{
    TrafficLightActuated::executeEachTimeStep(simulationDone);

    if(TLControlMode != TL_LowDelay)
        return;

    intervalElapseTime += updateInterval;
}


void TrafficLightLowDelay::chooseNextInterval()
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

        // update TL status for this phase
        updateTLstate("C", "red");
    }
    else if (currentInterval == "red")
    {
        currentInterval = nextGreenInterval;

        // set the new state
        TraCI->TLSetState("C", nextGreenInterval);
        intervalElapseTime = 0.0;
        intervalOffSet = minGreenTime;  // todo: assign green time dynamically

        // update TL status for this phase
        if(nextGreenInterval == firstGreen["C"])
            updateTLstate("C", "phaseEnd", nextGreenInterval, true);   // todo: the notion of cycle?
        else
            updateTLstate("C", "phaseEnd", nextGreenInterval);
    }
    else
        chooseNextGreenInterval();

    char buff[300];
    sprintf(buff, "SimTime: %4.2f | Planned interval: %s | Start time: %4.2f | End time: %4.2f", simTime().dbl(), currentInterval.c_str(), simTime().dbl(), simTime().dbl() + intervalOffSet);
    std::cout << buff << endl << endl;
}


void TrafficLightLowDelay::chooseNextGreenInterval()
{
    // clear the priority queue
    batchMovementDelay = std::priority_queue < batchMovementDelayEntry, std::vector<batchMovementDelayEntry>, movementCompareDelay >();

    // get which row has the highest delay
    for(unsigned int i = 0; i < allMovements.size(); ++i)  // row
    {
        int totalDelayRow = 0;
        int oneCount = 0;

        for(unsigned int linkNumber = 0; linkNumber < allMovements[i].size(); ++linkNumber)  // column (link number)
        {
            if(allMovements[i][linkNumber] == 1)
            {
                bool notRightTurn = std::find(std::begin(rightTurns), std::end(rightTurns), linkNumber) == std::end(rightTurns);
                // ignore this link if right turn
                if(notRightTurn)
                {
                    std::map<std::string /*vehID*/, double /*accum delay of vehID*/> vehs = linkDelay[std::make_pair("C",linkNumber)];

                    for(std::map<std::string, double>::iterator it = vehs.begin(); it != vehs.end(); ++it)
                        totalDelayRow = totalDelayRow + (*it).second;
                }

                oneCount++;
            }
        }

        // add this batch of movements to priority_queue
        batchMovementDelayEntry *entry = new batchMovementDelayEntry(oneCount, totalDelayRow, allMovements[i]);
        batchMovementDelay.push(*entry);
    }

    // get the movement batch with the highest delay
    batchMovementDelayEntry entry = batchMovementDelay.top();
    std::vector<int> batchMovements = entry.batchMovements;

    // calculate the next green interval.
    // right-turns are all permissive and are given 'g'
    nextGreenInterval = "";
    for(unsigned int linkNumber = 0; linkNumber < batchMovements.size(); ++linkNumber)
    {
        // if a right turn
        bool rightTurn = std::find(std::begin(rightTurns), std::end(rightTurns), linkNumber) != std::end(rightTurns);

        if(batchMovements[linkNumber] == 0)
            nextGreenInterval += 'r';
        else if(batchMovements[linkNumber] == 1 && rightTurn)
            nextGreenInterval += 'g';
        else
            nextGreenInterval += 'G';
    }

    // calculate 'next interval'
    std::string nextInterval = "";
    bool needYellowInterval = false;  // if we have at least one yellow interval
    for(unsigned int linkNumber = 0; linkNumber < batchMovements.size(); ++linkNumber)
    {
        if( (currentInterval[linkNumber] == 'G' || currentInterval[linkNumber] == 'g') && nextGreenInterval[linkNumber] == 'r')
        {
            nextInterval += 'y';
            needYellowInterval = true;
        }
        else
            nextInterval += currentInterval[linkNumber];
    }

    if(needYellowInterval)
    {
        currentInterval = "yellow";
        TraCI->TLSetState("C", nextInterval);

        intervalElapseTime = 0.0;
        intervalOffSet =  yellowTime;
    }
    else
    {
        intervalOffSet = minGreenTime;   // todo: assign green time extension dynamically
        std::cout << "Continue the last green interval." << endl;
    }


    //    // print debugging
    //    std::cout << endl;
    //    std::cout << "set of links with max q     ";
    //    for(int k =0; k < LINKSIZE; ++k)
    //        if(allMovements[row][k] == 1)
    //            std::cout << k << " (" << linkQueueSize[std::make_pair("C",k)] << "), ";
    //    std::cout << endl;




    // todo: for debugging
    //    for(std::map<std::string, std::map<std::string, double>>::iterator y = laneDelay.begin(); y != laneDelay.end(); ++y)
    //    {
    //        std::cout << (*y).first << ": ";
    //        std::map<std::string,double> inside = (*y).second;
    //
    //        for(std::map<std::string, double>::iterator z = inside.begin(); z != inside.end(); ++z)
    //        {
    //            std::cout << "(" << (*z).first << "," << (*z).second << ")";
    //        }
    //        std::cout << endl;
    //    }
    //    std::cout << endl;



    // debugging (print the first 10 entries)
    for(unsigned int o = 0; o < 10; ++o )
    {
        batchMovementDelayEntry entry = batchMovementDelay.top();
        std::cout << entry.totalDelay << ", " << entry.oneCount << ", ";

        int count = 0;
        for(std::vector<int>::iterator y = entry.batchMovements.begin(); y != entry.batchMovements.end(); ++y)
        {
            //std::cout << count << ":(" << *y << "," << linkQueueSize[std::make_pair("C",count)] << ")|";
            std::cout << *y;
            count++;
        }

        std::cout << endl;
        batchMovementDelay.pop();
    }

    std::cout << endl;

}

}

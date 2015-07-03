/****************************************************************************/
/// @file    TL_Fixed.cc
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

#include <04_TL_Fixed.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightFixed);


TrafficLightFixed::~TrafficLightFixed()
{

}


void TrafficLightFixed::initialize(int stage)
{
    VehDelay::initialize(stage);

    if(TLControlMode != TL_Fix_Time)
        return;

    if(stage == 0)
    {

    }
}


void TrafficLightFixed::finish()
{
    VehDelay::finish();
}


void TrafficLightFixed::handleMessage(cMessage *msg)
{
    VehDelay::handleMessage(msg);
}


void TrafficLightFixed::executeFirstTimeStep()
{
    // call parent
    VehDelay::executeFirstTimeStep();

    if (TLControlMode != TL_Fix_Time)
        return;

    std::cout << endl << "Fixed-time traffic signal control ... " << endl << endl;

    for (std::list<std::string>::iterator TL = TLList.begin(); TL != TLList.end(); ++TL)
    {
        TraCI->TLSetProgram(*TL, "fix-time1");

        if(collectTLData)
        {
            // initialize phase number in this TL
            phaseTL[*TL] = 1;

            // get all incoming lanes
            std::list<std::string> lan = TraCI->TLGetControlledLanes(*TL);

            // remove duplicate entries
            lan.unique();

            // Initialize status in this TL
            std::string currentInterval = TraCI->TLGetState(*TL);
            currentStatusTL *entry = new currentStatusTL(currentInterval, simTime().dbl(), -1, -1, -1, lan.size(), -1);
            statusTL.insert( std::make_pair(std::make_pair(*TL,1), *entry) );
        }
    }
}


void TrafficLightFixed::executeEachTimeStep(bool simulationDone)
{
    // call parent
    VehDelay::executeEachTimeStep(simulationDone);

    if (TLControlMode != TL_Fix_Time)
        return;

    int intervalNumber = TraCI->TLGetPhase("C");
    std::map<std::pair<std::string,int>, currentStatusTL>::iterator location = statusTL.find( std::make_pair("C",phaseTL["C"]) );

    // green interval
    if(intervalNumber % 3 == 0 && (location->second).redStart != -1)
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
        (location->second).phaseEnd = simTime().dbl();
        (location->second).totalQueueSize = totalQueueSize;

        // increase phase number by 1
        std::map<std::string, int>::iterator location2 = phaseTL.find("C");
        location2->second = location2->second + 1;

        // update status for the new phase
        std::string currentInterval = TraCI->TLGetState("C");
        currentStatusTL *entry = new currentStatusTL(currentInterval, simTime().dbl(), -1, -1, -1, lan.size(), -1);
        statusTL.insert( std::make_pair(std::make_pair("C",location2->second), *entry) );
    }
    // yellow interval
    else if(intervalNumber % 3 == 1 && (location->second).yellowStart == -1)
    {
        // update TL status for this phase
        (location->second).yellowStart = simTime().dbl();
    }
    // red interval
    else if(intervalNumber % 3 == 2 && (location->second).redStart == -1)
    {
        // update TL status for this phase
        (location->second).redStart = simTime().dbl();
    }
}

}

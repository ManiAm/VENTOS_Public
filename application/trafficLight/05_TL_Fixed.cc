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

#include <05_TL_Fixed.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightFixed);


TrafficLightFixed::~TrafficLightFixed()
{

}


void TrafficLightFixed::initialize(int stage)
{
    TrafficLightAllowedMoves::initialize(stage);

    if(TLControlMode != TL_Fix_Time)
        return;

    if(stage == 0)
    {

    }
}


void TrafficLightFixed::finish()
{
    TrafficLightAllowedMoves::finish();
}


void TrafficLightFixed::handleMessage(cMessage *msg)
{
    TrafficLightAllowedMoves::handleMessage(msg);
}


void TrafficLightFixed::executeFirstTimeStep()
{
    // call parent
    TrafficLightAllowedMoves::executeFirstTimeStep();

    if (TLControlMode != TL_Fix_Time)
        return;

    std::cout << "Fixed-time traffic signal control ... " << endl << endl;

    for (std::list<std::string>::iterator TL = TLList.begin(); TL != TLList.end(); ++TL)
    {
        TraCI->TLSetProgram(*TL, "fix-time1");

        // saving the first green interval in this TL. We need this to detect the cycle beginning.
        // NOTE: TL offset determines the first green interval.
        firstGreen[*TL] = TraCI->TLGetState(*TL);

        // initialize TL status
        updateTLstate(*TL, "init", firstGreen[*TL]);
    }
}


void TrafficLightFixed::executeEachTimeStep(bool simulationDone)
{
    // call parent
    TrafficLightAllowedMoves::executeEachTimeStep(simulationDone);

    if (TLControlMode != TL_Fix_Time)
        return;

    // updating TL status
    int intervalNumber = TraCI->TLGetPhase("C");

    std::map<std::pair<std::string,int>, currentStatusTL>::iterator location = statusTL.find( std::make_pair("C",phaseTL["C"]) );
    currentStatusTL stat = location->second;

    // current phase is ended. Green interval starts
    if(intervalNumber % 3 == 0 && stat.redStart != -1)
    {
        std::string currentInterval = TraCI->TLGetState("C"); // get the new green interval

        if(currentInterval == firstGreen["C"])
            updateTLstate("C", "phaseEnd", currentInterval, true /*new cycle*/);
        else
            updateTLstate("C", "phaseEnd", currentInterval);
    }
    // yellow interval starts
    else if(intervalNumber % 3 == 1 && stat.yellowStart == -1)
    {
        updateTLstate("C", "yellow");
    }
    // red interval starts
    else if(intervalNumber % 3 == 2 && stat.redStart == -1)
    {
        updateTLstate("C", "red");
    }
}

}

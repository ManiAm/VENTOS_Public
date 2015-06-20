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

#include <03_TL_Fixed.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightFixed);


TrafficLightFixed::~TrafficLightFixed()
{

}


void TrafficLightFixed::initialize(int stage)
{
    LoopDetectors::initialize(stage);

    if(TLControlMode != TL_Fix_Time)
        return;

    if(stage == 0)
    {

    }
}


void TrafficLightFixed::finish()
{
    LoopDetectors::finish();

}


void TrafficLightFixed::handleMessage(cMessage *msg)
{
    LoopDetectors::handleMessage(msg);

}


void TrafficLightFixed::executeFirstTimeStep()
{
    // call parent
    LoopDetectors::executeFirstTimeStep();

    if (TLControlMode != TL_Fix_Time)
        return;

    std::cout << "Fixed-time traffic signal control ... " << endl << endl;

    for (std::list<std::string>::iterator TL = TLList.begin(); TL != TLList.end(); TL++)
        TraCI->TLSetProgram(*TL, "fix-time");
}


void TrafficLightFixed::executeEachTimeStep(bool simulationDone)
{
    // call parent
    LoopDetectors::executeEachTimeStep(simulationDone);
}

}

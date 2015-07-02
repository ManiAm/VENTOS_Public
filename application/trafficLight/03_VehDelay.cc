/****************************************************************************/
/// @file    VehDelay.cc
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

#include <03_VehDelay.h>

namespace VENTOS {

Define_Module(VENTOS::VehDelay);


VehDelay::~VehDelay()
{

}


void VehDelay::initialize(int stage)
{
    LoopDetectors::initialize(stage);

    if(stage == 0)
    {
        measureVehDelay = par("measureVehDelay").boolValue();
    }
}


void VehDelay::finish()
{
    LoopDetectors::finish();
}


void VehDelay::handleMessage(cMessage *msg)
{
    LoopDetectors::handleMessage(msg);
}


void VehDelay::executeFirstTimeStep()
{
    // call parent
    LoopDetectors::executeFirstTimeStep();

    // get all lanes in the network
    lanesList = TraCI->laneGetIDList();
}


void VehDelay::executeEachTimeStep(bool simulationDone)
{
    // call parent
    LoopDetectors::executeEachTimeStep(simulationDone);

    if(measureVehDelay)
    {
        vehiclesDelay();

        if(ev.isGUI()) vehiclesDelayToFile();   // (if in GUI) write what we have collected so far
        else if(simulationDone) vehiclesDelayToFile();  // (if in CMD) write to file at the end of simulation
    }
}


void VehDelay::vehiclesDelay()
{
    for(std::list<std::string>::iterator i = lanesList.begin(); i != lanesList.end(); ++i)
    {
        // get all vehicles on lane i
        std::list<std::string> allVeh = TraCI->laneGetLastStepVehicleIDs( i->c_str() );

        for(std::list<std::string>::reverse_iterator k = allVeh.rbegin(); k != allVeh.rend(); ++k)
            vehiclesDelayEach(k->c_str());
    }
}


void VehDelay::vehiclesDelayEach(std::string vID)
{
    // look for the vehicle in delay map
    std::map<std::string, delayEntry>::iterator loc = delay.find(vID);

}


void VehDelay::vehiclesDelayToFile()
{

}


}

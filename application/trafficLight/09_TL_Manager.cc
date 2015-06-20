/****************************************************************************/
/// @file    TL_Manager.cc
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

#include <09_TL_Manager.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightManager);


TrafficLightManager::~TrafficLightManager()
{

}


void TrafficLightManager::initialize(int stage)
{
    TrafficLightRouter::initialize(stage);

    if(stage == 0)
    {

    }
}


void TrafficLightManager::finish()
{
    TrafficLightRouter::finish();

}


void TrafficLightManager::handleMessage(cMessage *msg)
{
    TrafficLightRouter::handleMessage(msg);

}


void TrafficLightManager::executeFirstTimeStep()
{
    // call parent
    TrafficLightRouter::executeFirstTimeStep();

    // check if the TLControlMode number is valid?
    if(TLControlMode < 0 || TLControlMode > 6)
    {
        error("Invalid TLControlMode!");
    }
}


void TrafficLightManager::executeEachTimeStep(bool simulationDone)
{
    // call parent
    TrafficLightRouter::executeEachTimeStep(simulationDone);
}

}

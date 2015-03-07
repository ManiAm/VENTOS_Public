/****************************************************************************/
/// @file    ApplRSU_03_Manager.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
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

#include "ApplRSU_03_Manager.h"

namespace VENTOS {

Define_Module(VENTOS::ApplRSUManager);

ApplRSUManager::~ApplRSUManager()
{

}


void ApplRSUManager::initialize(int stage)
{
	ApplRSUAID::initialize(stage);

	if (stage==0)
	{

	}
}


void ApplRSUManager::finish()
{
    ApplRSUAID::finish();
}


void ApplRSUManager::handleSelfMsg(cMessage* msg)
{
    ApplRSUAID::handleSelfMsg(msg);

}


void ApplRSUManager::handleLowerMsg(cMessage* msg)
{
    // make sure msg is of type WaveShortMessage
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (string(wsm->getName()) == "beaconVehicle")
    {
        BeaconVehicle* wsm = dynamic_cast<BeaconVehicle*>(msg);
        ASSERT(wsm);

        ApplRSUManager::onBeaconVehicle(wsm);
    }
    else if (string(wsm->getName()) == "beaconBicycle")
    {
        BeaconBicycle* wsm = dynamic_cast<BeaconBicycle*>(msg);
        ASSERT(wsm);

        ApplRSUManager::onBeaconBicycle(wsm);
    }
    else if (string(wsm->getName()) == "beaconPedestrian")
    {
        BeaconPedestrian* wsm = dynamic_cast<BeaconPedestrian*>(msg);
        ASSERT(wsm);

        ApplRSUManager::onBeaconPedestrian(wsm);
    }
    else if (string(wsm->getName()) == "beaconRSU")
    {
        BeaconRSU* wsm = dynamic_cast<BeaconRSU*>(msg);
        ASSERT(wsm);

        ApplRSUManager::onBeaconRSU(wsm);
    }
    else if(string(wsm->getName()) == "laneChangeMsg")
    {
        LaneChangeMsg* wsm = dynamic_cast<LaneChangeMsg*>(msg);
        ASSERT(wsm);

        ApplRSUManager::onData(wsm);
    }

    delete msg;
}


void ApplRSUManager::onBeaconVehicle(BeaconVehicle* wsm)
{
    ApplRSUAID::onBeaconVehicle(wsm);
}


void ApplRSUManager::onBeaconBicycle(BeaconBicycle* wsm)
{
    ApplRSUAID::onBeaconBicycle(wsm);
}


void ApplRSUManager::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    ApplRSUAID::onBeaconPedestrian(wsm);
}


void ApplRSUManager::onBeaconRSU(BeaconRSU* wsm)
{
    ApplRSUAID::onBeaconRSU(wsm);
}


void ApplRSUManager::onData(LaneChangeMsg* wsm)
{
    ApplRSUAID::onData(wsm);
}

}


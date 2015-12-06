/****************************************************************************/
/// @file    ApplPed_03_Manager.cc
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

#include "ApplPed_03_Manager.h"

namespace VENTOS {

Define_Module(VENTOS::ApplPedManager);

ApplPedManager::~ApplPedManager()
{

}


void ApplPedManager::initialize(int stage)
{
    ApplPedBeacon::initialize(stage);

	if (stage == 0)
	{

	}
}


void ApplPedManager::finish()
{
    ApplPedBeacon::finish();
}


void ApplPedManager::handleSelfMsg(cMessage* msg)
{
    ApplPedBeacon::handleSelfMsg(msg);
}


void ApplPedManager::handleLowerMsg(cMessage* msg)
{
    // make sure msg is of type WaveShortMessage
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()) == "beaconVehicle")
    {
        BeaconVehicle* wsm = dynamic_cast<BeaconVehicle*>(msg);
        ASSERT(wsm);
    }
    else if (std::string(wsm->getName()) == "beaconRSU")
    {
        BeaconRSU* wsm = dynamic_cast<BeaconRSU*>(msg);
        ASSERT(wsm);
    }
    else if(std::string(wsm->getName()) == "platoonMsg")
    {
        PlatoonMsg* wsm = dynamic_cast<PlatoonMsg*>(msg);
        ASSERT(wsm);

        ApplPedManager::onData(wsm);
    }

    delete msg;
}


void ApplPedManager::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down
    //ApplPedBeacon::onBeaconVehicle(wsm);
}


void ApplPedManager::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down
    //ApplPedBeacon::onBeaconRSU(wsm);
}


void ApplPedManager::onData(PlatoonMsg* wsm)
{
    // pass it down
    //ApplPedBeacon::onData(wsm);
}


// is called, every time the position of vehicle changes
void ApplPedManager::handlePositionUpdate(cObject* obj)
{
    // pass it down
    ApplPedBeacon::handlePositionUpdate(obj);
}


}


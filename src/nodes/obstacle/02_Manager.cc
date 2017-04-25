/****************************************************************************/
/// @file    Manager.cc
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

#include "nodes/obstacle/02_Manager.h"

namespace VENTOS {

Define_Module(VENTOS::ApplObstacleManager);

ApplObstacleManager::~ApplObstacleManager()
{

}


void ApplObstacleManager::initialize(int stage)
{
    super::initialize(stage);

	if (stage == 0)
	{

	}
}


void ApplObstacleManager::finish()
{
    super::finish();
}


void ApplObstacleManager::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


void ApplObstacleManager::handleLowerMsg(omnetpp::cMessage* msg)
{
    // Only DSRC-enabled obstacles accept this msg
    if(!DSRCenabled)
    {
        delete msg;
        return;
    }

    if (msg->getKind() == TYPE_BEACON_VEHICLE)
    {
        BeaconVehicle* wsm = dynamic_cast<BeaconVehicle*>(msg);
        ASSERT(wsm);
    }
    else if (msg->getKind() == TYPE_BEACON_RSU)
    {
        BeaconRSU* wsm = dynamic_cast<BeaconRSU*>(msg);
        ASSERT(wsm);
    }

    delete msg;
}


void ApplObstacleManager::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down
    //super::onBeaconVehicle(wsm);

}


void ApplObstacleManager::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down
    //super::onBeaconRSU(wsm);
}

}


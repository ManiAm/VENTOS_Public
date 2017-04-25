/****************************************************************************/
/// @file    Beacon.cc
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

#include "nodes/obstacle/01_Beacon.h"

namespace VENTOS {

Define_Module(VENTOS::ApplObstacleBeacon);

ApplObstacleBeacon::~ApplObstacleBeacon()
{

}


void ApplObstacleBeacon::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {

    }
}


void ApplObstacleBeacon::finish()
{
    super::finish();
}


void ApplObstacleBeacon::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


void ApplObstacleBeacon::sendBeacon()
{
    BeaconObstacle* beaconMsg = generateBeacon();

    // broadcast the beacon wirelessly using IEEE 802.11p
    send(beaconMsg, lowerLayerOut);
}


BeaconObstacle*  ApplObstacleBeacon::generateBeacon()
{
    BeaconObstacle* wsm = new BeaconObstacle("beaconObstacle", TYPE_BEACON_OBSTACLE);

    // add header length
    wsm->addBitLength(headerLength);

    // add payload length
    wsm->addBitLength(beaconLengthBits);

    wsm->setWsmVersion(1);
    wsm->setSecurityType(1);

    wsm->setChannelNumber(Veins::Channels::CCH);

    wsm->setDataRate(1);
    wsm->setPriority(beaconPriority);
    wsm->setPsid(0);

    // fill in the sender/receiver fields
    wsm->setSender(SUMOID.c_str());
    wsm->setSenderType(SUMOType.c_str());

    // set current position
    TraCICoord cord = TraCI->vehicleGetPosition(SUMOID);
    wsm->setPos(cord);

    // set current lane
    wsm->setLane( TraCI->vehicleGetLaneID(SUMOID).c_str() );

    return wsm;
}

}


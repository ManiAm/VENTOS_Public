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

#include "obstacle/02_Beacon.h"

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
        // NED
        DSRCenabled = getParentModule()->par("DSRCenabled").boolValue();

        // NED variables (beaconing parameters)
        sendBeacons = par("sendBeacons").boolValue();
        beaconInterval = par("beaconInterval").doubleValue();
        maxOffset = par("maxOffset").doubleValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        // NED variables (data parameters)
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        // simulate asynchronous channel access
        double offSet = dblrand() * (beaconInterval/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        ObstacleBeaconEvt = new omnetpp::cMessage("BeaconEvt", TYPE_TIMER);
        if (DSRCenabled)
            scheduleAt(omnetpp::simTime() + offSet, ObstacleBeaconEvt);
    }
}


void ApplObstacleBeacon::finish()
{
    super::finish();

    if (ObstacleBeaconEvt->isScheduled())
        cancelAndDelete(ObstacleBeaconEvt);
    else
        delete ObstacleBeaconEvt;
}


void ApplObstacleBeacon::handleSelfMsg(omnetpp::cMessage* msg)
{
    if (msg == ObstacleBeaconEvt)
    {
        // make sure DSRCenabled is true
        if(DSRCenabled && sendBeacons)
        {
            BeaconObstacle* beaconMsg = generateBeacon();

            // broadcast the beacon wirelessly using IEEE 802.11p
            sendDelayed(beaconMsg, individualOffset, lowerLayerOut);
        }

        // schedule for next beacon broadcast
        scheduleAt(omnetpp::simTime() + beaconInterval, ObstacleBeaconEvt);
    }
    else
        super::handleSelfMsg(msg);
}


BeaconObstacle*  ApplObstacleBeacon::generateBeacon()
{
    BeaconObstacle* wsm = new BeaconObstacle("beaconObstacle", TYPE_BEACON_BICYCLE);

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
    Coord cord = TraCI->vehicleGetPosition(SUMOID);
    wsm->setPos(cord);

    // set current lane
    wsm->setLane( TraCI->vehicleGetLaneID(SUMOID).c_str() );

    return wsm;
}


// is called, every time the position of bicycle changes
void ApplObstacleBeacon::handlePositionUpdate(cObject* obj)
{
    super::handlePositionUpdate(obj);
}

}


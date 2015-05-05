/****************************************************************************/
/// @file    ApplPed_02_Beacon.cc
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

#include "ApplPed_02_Beacon.h"

namespace VENTOS {

Define_Module(VENTOS::ApplPedBeacon);

ApplPedBeacon::~ApplPedBeacon()
{

}


void ApplPedBeacon::initialize(int stage)
{
    ApplPedBase::initialize(stage);

	if (stage == 0)
	{
	    // NED
        VANETenabled = par("VANETenabled").boolValue();

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

        PedestrianBeaconEvt = new cMessage("BeaconEvt", KIND_TIMER);
        if (VANETenabled)
        {
            scheduleAt(simTime() + offSet, PedestrianBeaconEvt);
        }
	}
}


void ApplPedBeacon::finish()
{
    ApplPedBase::finish();

    if (PedestrianBeaconEvt->isScheduled())
    {
        cancelAndDelete(PedestrianBeaconEvt);
    }
    else
    {
        delete PedestrianBeaconEvt;
    }
}


void ApplPedBeacon::handleSelfMsg(cMessage* msg)
{
    ApplPedBase::handleSelfMsg(msg);

    if (msg == PedestrianBeaconEvt)
    {
        if(VANETenabled && sendBeacons)
        {
            BeaconPedestrian* beaconMsg = ApplPedBeacon::prepareBeacon();

            EV << "## Created beacon msg for pedestrian: " << SUMOpID << endl;
            ApplPedBeacon::printBeaconContent(beaconMsg);

            // send it
            sendDelayed(beaconMsg, individualOffset, lowerLayerOut);
        }

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, PedestrianBeaconEvt);
    }
}


BeaconPedestrian*  ApplPedBeacon::prepareBeacon()
{
    BeaconPedestrian* wsm = new BeaconPedestrian("beaconPedestrian");

    // add header length
    wsm->addBitLength(headerLength);

    // add payload length
    wsm->addBitLength(beaconLengthBits);

    wsm->setWsmVersion(1);
    wsm->setSecurityType(1);

    wsm->setChannelNumber(Channels::CCH);

    wsm->setDataRate(1);
    wsm->setPriority(beaconPriority);
    wsm->setPsid(0);

    // wsm->setSerial(serial);
    // wsm->setTimestamp(simTime());

    // fill in the sender/receiver fields
    wsm->setSender(SUMOpID.c_str());
    wsm->setRecipient("broadcast");

    // set current position
    Coord cord = TraCI->personGetPosition(SUMOpID);
    wsm->setPos(cord);

    // set current speed
    wsm->setSpeed( TraCI->personGetSpeed(SUMOpID) );

    // set current acceleration
    wsm->setAccel( -1 );

    // set maxDecel
    wsm->setMaxDecel( -1 );

    // set current lane
    wsm->setLane( TraCI->personGetEdgeID(SUMOpID).c_str() );

    return wsm;
}


// print beacon fields (for debugging purposes)
void ApplPedBeacon::printBeaconContent(BeaconPedestrian* wsm)
{
    EV << wsm->getWsmVersion() << " | ";
    EV << wsm->getSecurityType() << " | ";
    EV << wsm->getChannelNumber() << " | ";
    EV << wsm->getDataRate() << " | ";
    EV << wsm->getPriority() << " | ";
    EV << wsm->getPsid() << " | ";
    EV << wsm->getPsc() << " | ";
    EV << wsm->getWsmLength() << " | ";
    EV << wsm->getWsmData() << " ||| ";

    EV << wsm->getSender() << " | ";
    EV << wsm->getRecipient() << " | ";
    EV << wsm->getPos() << " | ";
    EV << wsm->getSpeed() << " | ";
    EV << wsm->getAccel() << " | ";
    EV << wsm->getMaxDecel() << " | ";
    EV << wsm->getLane() << " | ";
    EV << wsm->getPlatoonID() << " | ";
    EV << wsm->getPlatoonDepth() << endl;
}


// is called, every time the position of pedestrian changes
void ApplPedBeacon::handlePositionUpdate(cObject* obj)
{
    ApplPedBase::handlePositionUpdate(obj);
}

}


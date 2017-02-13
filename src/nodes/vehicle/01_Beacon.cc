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

#include <vehicle/01_Beacon.h>

namespace VENTOS {

Define_Module(VENTOS::ApplVBeacon);

ApplVBeacon::~ApplVBeacon()
{

}


void ApplVBeacon::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        signalBeaconing = par("signalBeaconing").boolValue();
        sonarDist = par("sonarDist").doubleValue();

        WATCH(plnID);
        WATCH(myPlnDepth);
        WATCH(plnSize);
    }
}


void ApplVBeacon::finish()
{
    super::finish();
}


void ApplVBeacon::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


void ApplVBeacon::sendBeacon()
{
    BeaconVehicle* beaconMsg = generateBeacon();

    // fill-in the related fields to platoon
    beaconMsg->setPlatoonID(plnID.c_str());
    beaconMsg->setPlatoonDepth(myPlnDepth);

    // send the beacon as a signal. Any module registered to this signal can
    // receive a copy of the beacon (for now, only RSUs are registered)
    if(signalBeaconing)
    {
        omnetpp::simsignal_t Signal_beaconSignaling = registerSignal("beaconSignaling");
        this->getParentModule()->emit(Signal_beaconSignaling, beaconMsg);
    }
    // broadcast the beacon wirelessly using IEEE 802.11p
    else
        send(beaconMsg, lowerLayerOut);
}


BeaconVehicle*  ApplVBeacon::generateBeacon()
{
    BeaconVehicle* wsm = new BeaconVehicle("beaconVehicle", TYPE_BEACON_VEHICLE);

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
    // wsm->setSerial(serial);
    // wsm->setTimestamp(simTime());

    // fill in the sender/receiver fields
    wsm->setSender(SUMOID.c_str());
    wsm->setSenderType(SUMOType.c_str());
    wsm->setRecipient("broadcast");

    // set current position
    TraCICoord cord = TraCI->vehicleGetPosition(SUMOID);
    wsm->setPos(cord);

    // set current speed
    wsm->setSpeed( TraCI->vehicleGetSpeed(SUMOID) );

    // set current acceleration
    wsm->setAccel( TraCI->vehicleGetCurrentAccel(SUMOID) );

    // set maxDecel
    wsm->setMaxDecel( TraCI->vehicleGetMaxDecel(SUMOID) );

    // set current lane
    wsm->setLane( TraCI->vehicleGetLaneID(SUMOID).c_str() );

    // set heading -- used in rsu/classify beacons
    wsm->setAngle( TraCI->vehicleGetAngle(SUMOID) );

    return wsm;
}


bool ApplVBeacon::isBeaconFromLeading(BeaconVehicle* wsm)
{
    auto leader = TraCI->vehicleGetLeader(SUMOID, sonarDist);

    if( leader.leaderID == std::string(wsm->getSender()) )
        return true;
    else
        return false;
}


bool ApplVBeacon::isBeaconFromMyPlatoonLeader(BeaconVehicle* wsm)
{
    // check if a platoon leader is sending this
    if( wsm->getPlatoonDepth() == 0 )
    {
        // check if this is my platoon leader
        if( std::string(wsm->getPlatoonID()) == plnID)
        {
            // note: we should not check myPlnDepth != 0
            // in predefined platoon, we do not use depth!
            return true;
        }
    }

    return false;
}

}


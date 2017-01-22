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

#include "vehicle/02_Beacon.h"

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
        // NED
        DSRCenabled = getParentModule()->par("DSRCenabled").boolValue();
        sonarDist = par("sonarDist").doubleValue();

        // NED variables (beaconing parameters)
        sendBeacons = par("sendBeacons").boolValue();
        beaconInterval = par("beaconInterval").doubleValue();
        maxOffset = par("maxOffset").doubleValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        // NED variables
        signalBeaconing = par("signalBeaconing").boolValue();

        // NED variables (data parameters)
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        // simulate asynchronous channel access
        double offSet = dblrand() * (beaconInterval/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        vehicleBeaconEvt = new omnetpp::cMessage("BeaconEvt", TYPE_TIMER);
        if (DSRCenabled)
            scheduleAt(omnetpp::simTime() + offSet, vehicleBeaconEvt);

        plnID = "";
        myPlnDepth = -1;
        plnSize = -1;
        plnMembersList.clear();

        WATCH(plnID);
        WATCH(myPlnDepth);
        WATCH(plnSize);
        WATCH(DSRCenabled);
    }
}


void ApplVBeacon::finish()
{
    super::finish();

    if (vehicleBeaconEvt->isScheduled())
        cancelAndDelete(vehicleBeaconEvt);
    else
        delete vehicleBeaconEvt;
}


void ApplVBeacon::handleSelfMsg(omnetpp::cMessage* msg)
{
    if (msg == vehicleBeaconEvt)
    {
        // make sure DSRCenabled is true
        if(DSRCenabled && sendBeacons)
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
                sendDelayed(beaconMsg, individualOffset, lowerLayerOut);
        }

        // schedule for next beacon broadcast
        scheduleAt(omnetpp::simTime() + beaconInterval, vehicleBeaconEvt);
    }
    else
        super::handleSelfMsg(msg);
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
    Coord cord = TraCI->vehicleGetPosition(SUMOID);
    wsm->setPos(cord);

    // set current speed
    wsm->setSpeed( TraCI->vehicleGetSpeed(SUMOID) );

    // set current acceleration
    wsm->setAccel( TraCI->vehicleGetCurrentAccel(SUMOID) );

    // set maxDecel
    wsm->setMaxDecel( TraCI->vehicleGetMaxDecel(SUMOID) );

    // set current lane
    wsm->setLane( TraCI->vehicleGetLaneID(SUMOID).c_str() );

    // set heading
    wsm->setAngle( TraCI->vehicleGetAngle(SUMOID) );

    return wsm;
}


// is called, every time the position of vehicle changes
void ApplVBeacon::handlePositionUpdate(cObject* obj)
{
    super::handlePositionUpdate(obj);
}


bool ApplVBeacon::isBeaconFromLeading(BeaconVehicle* wsm)
{
    std::vector<std::string> vleaderIDnew = TraCI->vehicleGetLeader(SUMOID, sonarDist);
    std::string vleaderID = vleaderIDnew[0];

    if( vleaderID == std::string(wsm->getSender()) )
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


/****************************************************************************/
/// @file    ApplV_02_Beacon.cc
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

#include "ApplV_02_Beacon.h"

namespace VENTOS {

Define_Module(VENTOS::ApplVBeacon);

ApplVBeacon::~ApplVBeacon()
{

}


void ApplVBeacon::initialize(int stage)
{
    ApplVBase::initialize(stage);

    if (stage == 0)
    {
        // NED
        sonarDist = par("sonarDist").doubleValue();

        // NED variables (beaconing parameters)
        sendBeacons = par("sendBeacons").boolValue();
        beaconInterval = par("beaconInterval").doubleValue();
        maxOffset = par("maxOffset").doubleValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        // NED variables
        smartBeaconing = par("smartBeaconing").boolValue();
        signalBeaconing = par("signalBeaconing").boolValue();

        // NED variables (data parameters)
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        // simulate asynchronous channel access
        double offSet = dblrand() * (beaconInterval/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        if(SUMOvType == "TypeObstacle")
            VANETenabled = false;
        else
            VANETenabled = par("VANETenabled").boolValue();

        VehicleBeaconEvt = new cMessage("BeaconEvt", KIND_TIMER);
        if (VANETenabled)
            scheduleAt(simTime() + offSet, VehicleBeaconEvt);

        plnID = "";
        myPlnDepth = -1;
        plnSize = -1;
        plnMembersList.clear();

        WATCH(plnID);
        WATCH(myPlnDepth);
        WATCH(plnSize);
        WATCH(VANETenabled);

        crossing = false;
        leaving = false;
    }
}


void ApplVBeacon::finish()
{
    ApplVBase::finish();

    if (VehicleBeaconEvt->isScheduled())
    {
        cancelAndDelete(VehicleBeaconEvt);
    }
    else
    {
        delete VehicleBeaconEvt;
    }
}


void ApplVBeacon::handleSelfMsg(cMessage* msg)
{
    ApplVBase::handleSelfMsg(msg);

    if (msg == VehicleBeaconEvt)
    {
        // make sure VANETenabled is true
        if(VANETenabled)
        {
            if(smartBeaconing && TLControlMode == TL_VANET)
                smartBeaconingDecision();

            if(sendBeacons)
            {
                BeaconVehicle* beaconMsg = prepareBeacon();

                // fill-in the related fields to platoon
                beaconMsg->setPlatoonID(plnID.c_str());
                beaconMsg->setPlatoonDepth(myPlnDepth);

                // send the beacon as a signal. Any module registered to this signal can
                // receive a copy of the beacon (for now, only RSUs are registered)
                if(signalBeaconing)
                {
                    simsignal_t Signal_beaconSignaling = registerSignal("beaconSignaling");
                    nodePtr->emit(Signal_beaconSignaling, beaconMsg);
                }
                // broadcast the beacon wirelessly using IEEE 802.11p
                else
                    sendDelayed(beaconMsg, individualOffset, lowerLayerOut);
            }
        }

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, VehicleBeaconEvt);
    }
}


// the decision of beaconing or not depends on the current location of vehicle
void ApplVBeacon::smartBeaconingDecision()
{
    Coord myPos = TraCI->vehicleGetPosition(SUMOvID);

    // vehicle enters the zone
    // todo: change from fixed coordinates
    // coordinates should be a little bigger than the detection region
    // the vehicle should start beaconing a little bit sooner
    if( (myPos.x >= 830) && (myPos.x <= 960) && (myPos.y >= 830) && (myPos.y <= 960) )
    {
        // get the current edge
        std::string myEdge = TraCI->vehicleGetEdgeID(SUMOvID);

        // started to cross
        if( !crossing && (myEdge[0] == ':') && (myEdge[1] == 'C') )
        {
            crossing = true;
            sendBeacons = true;   // keep beaconing 'on' during crossing
        }
        // crossed the intersection
        else if( crossing && ((myEdge[0] != ':') || (myEdge[1] != 'C')) )
        {
            crossing = false;
            leaving = true;
            sendBeacons = false;
        }
        else if(leaving)
        {
            sendBeacons = false;
        }
        // not crossed yet or during crossing
        else
        {
            sendBeacons = true;
        }
    }
    else
        sendBeacons = false;
}


BeaconVehicle*  ApplVBeacon::prepareBeacon()
{
    BeaconVehicle* wsm = new BeaconVehicle("beaconVehicle");

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
    wsm->setSender(SUMOvID.c_str());
    wsm->setRecipient("broadcast");

    // set current position
    Coord cord = TraCI->vehicleGetPosition(SUMOvID);
    wsm->setPos(cord);

    // set current speed
    wsm->setSpeed( TraCI->vehicleGetSpeed(SUMOvID) );

    // set current acceleration
    wsm->setAccel( TraCI->vehicleGetCurrentAccel(SUMOvID) );

    // set maxDecel
    wsm->setMaxDecel( TraCI->vehicleGetMaxDecel(SUMOvID) );

    // set current lane
    wsm->setLane( TraCI->vehicleGetLaneID(SUMOvID).c_str() );

    return wsm;
}


// is called, every time the position of vehicle changes
void ApplVBeacon::handlePositionUpdate(cObject* obj)
{
    ApplVBase::handlePositionUpdate(obj);
}


bool ApplVBeacon::isBeaconFromLeading(BeaconVehicle* wsm)
{
    std::vector<std::string> vleaderIDnew = TraCI->vehicleGetLeader(SUMOvID, sonarDist);
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
        // check if this is actually my platoon leader
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


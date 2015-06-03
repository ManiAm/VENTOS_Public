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
        if(SUMOvType != "TypeObstacle")
        {
            VANETenabled = par("VANETenabled").boolValue();
        }
        else
        {
            VANETenabled = false;
        }

        sonarDist = par("sonarDist").doubleValue();

        // NED variables (beaconing parameters)
        sendBeacons = par("sendBeacons").boolValue();
        beaconInterval = par("beaconInterval").doubleValue();
        maxOffset = par("maxOffset").doubleValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        // NED variables
        smartBeaconing = par("smartBeaconing").boolValue();
        cModule *module = simulation.getSystemModule()->getSubmodule("TrafficLight");
        TLControlMode = module->par("TLControlMode").longValue();

        // NED variables (data parameters)
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        // simulate asynchronous channel access
        double offSet = dblrand() * (beaconInterval/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

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

        hasEntered = false;
        hasLeft = false;
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
        if(VANETenabled && smartBeaconing)
        {
            if(TLControlMode == 5)
            {
                Coord myPos = TraCI->vehicleGetPosition(SUMOvID);
                std::string myEdge = TraCI->vehicleGetEdgeID(SUMOvID);
                // todo: change from fixed coordinates
                if((!hasEntered) && (myPos.x > 350) && (myPos.x < 450) && (myPos.y > 350) && (myPos.y < 450))
                {
                    hasEntered = true;
                    sendBeacons = true;
                }
                else if((!hasLeft) && (myEdge[0] == ':') && (myEdge[1] == 'C'))
                {
                    hasLeft = true;
                    sendBeacons = true;
                }
                else
                    sendBeacons = false;
            }
            // turn off beaconing for any other TL controller
            else
                sendBeacons = false;
        }

        if(VANETenabled && sendBeacons)
        {
            BeaconVehicle* beaconMsg = prepareBeacon();

            // fill-in the related fields to platoon
            beaconMsg->setPlatoonID(plnID.c_str());
            beaconMsg->setPlatoonDepth(myPlnDepth);

            // send it
            sendDelayed(beaconMsg, individualOffset, lowerLayerOut);
        }

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, VehicleBeaconEvt);
    }
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


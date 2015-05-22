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
        {
            scheduleAt(simTime() + offSet, VehicleBeaconEvt);
        }

        plnID = "";
        myPlnDepth = -1;
        plnSize = -1;
        plnMembersList.clear();

        WATCH(plnID);
        WATCH(myPlnDepth);
        WATCH(plnSize);
        WATCH(VANETenabled);
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
        if(VANETenabled && sendBeacons)
        {
            BeaconVehicle* beaconMsg = prepareBeacon();

            // fill-in the related fields to platoon
            beaconMsg->setPlatoonID(plnID.c_str());
            beaconMsg->setPlatoonDepth(myPlnDepth);

            EV << "## Created beacon msg for vehicle: " << SUMOvID << endl;
            ApplVBeacon::printBeaconContent(beaconMsg);

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


// print beacon fields (for debugging purposes)
void ApplVBeacon::printBeaconContent(BeaconVehicle* wsm)
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


// is called, every time the position of vehicle changes
void ApplVBeacon::handlePositionUpdate(cObject* obj)
{
    ApplVBase::handlePositionUpdate(obj);
}


bool ApplVBeacon::isBeaconFromLeading(BeaconVehicle* wsm)
{
    // step 1: check if a leading vehicle is present

    std::vector<std::string> vleaderIDnew = TraCI->vehicleGetLeader(SUMOvID, sonarDist);
    std::string vleaderID = vleaderIDnew[0];

    if( vleaderID == std::string(wsm->getSender()) )
        return true;
    else
        return false;

    /*
    double gap = atof( vleaderIDnew[1].c_str() );

    if(vleaderID == "")
    {
        EV << "This vehicle has no leading vehicle." << endl;
        return false;
    }

    // step 2: is it on the same lane?

    string myLane = TraCI->commandGetVehicleLaneId(SUMOvID);
    string beaconLane = wsm->getLane();

    EV << "I am on lane " << TraCI->commandGetVehicleLaneId(SUMOvID) << ", and other vehicle is on lane " << wsm->getLane() << endl;

    if( myLane != beaconLane )
    {
        EV << "Not on the same lane!" << endl;
        return false;
    }

    EV << "We are on the same lane!" << endl;

    // step 3: is the distance equal to gap?

    Coord cord = TraCI->commandGetVehiclePos(SUMOvID);
    double dist = sqrt( pow(cord.x - wsm->getPos().x, 2) +  pow(cord.y - wsm->getPos().y, 2) );

    // subtract the length of the leading vehicle from dist
    dist = dist - TraCI->commandGetVehicleLength(vleaderID);

    EV << "my coord (x,y): " << cord.x << "," << cord.y << endl;
    EV << "other coord (x,y): " << wsm->getPos().x << "," << wsm->getPos().y << endl;
    EV << "distance is " << dist << ", and gap is " << gap << endl;

    double diff = fabs(dist - gap);

    if(diff > 0.001)
    {
        EV << "distance does not match the gap!" << endl;
        return false;
    }

    if(cord.x > wsm->getPos().x)
    {
        EV << "beacon is coming from behind!" << endl;
        return false;
    }

    EV << "This beacon is from the leading vehicle!" << endl;
    return true;
    */
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


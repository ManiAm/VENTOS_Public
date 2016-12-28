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

#include "bicycle/02_Beacon.h"

namespace VENTOS {

Define_Module(VENTOS::ApplBikeBeacon);

ApplBikeBeacon::~ApplBikeBeacon()
{

}


void ApplBikeBeacon::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        // NED
        DSRCenabled = par("DSRCenabled").boolValue();

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

        BicycleBeaconEvt = new omnetpp::cMessage("BeaconEvt", TYPE_TIMER);
        if (DSRCenabled)
            scheduleAt(omnetpp::simTime() + offSet, BicycleBeaconEvt);
    }
}


void ApplBikeBeacon::finish()
{
    super::finish();

    if (BicycleBeaconEvt->isScheduled())
        cancelAndDelete(BicycleBeaconEvt);
    else
        delete BicycleBeaconEvt;
}


void ApplBikeBeacon::handleSelfMsg(omnetpp::cMessage* msg)
{
    if (msg == BicycleBeaconEvt)
    {
        // make sure DSRCenabled is true
        if(DSRCenabled && sendBeacons)
        {
            BeaconBicycle* beaconMsg = generateBeacon();

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
        scheduleAt(omnetpp::simTime() + beaconInterval, BicycleBeaconEvt);
    }
    else
        super::handleSelfMsg(msg);
}


BeaconBicycle*  ApplBikeBeacon::generateBeacon()
{
    BeaconBicycle* wsm = new BeaconBicycle("beaconBicycle", TYPE_BEACON_BICYCLE);

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

    return wsm;
}


// is called, every time the position of bicycle changes
void ApplBikeBeacon::handlePositionUpdate(cObject* obj)
{
    super::handlePositionUpdate(obj);
}

}


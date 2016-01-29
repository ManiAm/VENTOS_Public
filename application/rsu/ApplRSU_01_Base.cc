/****************************************************************************/
/// @file    ApplRSU_01_Base.cc
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

#include "ApplRSU_01_Base.h"

namespace VENTOS {

Define_Module(VENTOS::ApplRSUBase);

ApplRSUBase::~ApplRSUBase()
{

}


void ApplRSUBase::initialize(int stage)
{
	BaseApplLayer::initialize(stage);

	if (stage==0)
	{
        // get the ptr of the current module
        nodePtr = this->getParentModule();
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        Signal_executeEachTS = registerSignal("executeEachTS");
        simulation.getSystemModule()->subscribe("executeEachTS", this);

        headerLength = par("headerLength").longValue();

        // NED variables (beaconing parameters)
        sendBeacons = par("sendBeacons").boolValue();
        beaconInterval = par("beaconInterval").doubleValue();
        maxOffset = par("maxOffset").doubleValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        // id in omnet++
		myId = getParentModule()->getIndex();
		myFullId = getParentModule()->getFullName();

        myTLid = par("myTLid").stringValue();   // TLid that this RSU belongs to (this parameter is set by AddRSU)
                                                // empty string means this RSU is not associated with any TL

        // my X coordinate in SUMO
        myCoordX = par("myCoordX").doubleValue();
        // my Y coordinate in SUMO
        myCoordY = par("myCoordY").doubleValue();

        // get a pointer to the TrafficLight module
        cModule *tmodule = simulation.getSystemModule()->getSubmodule("TrafficLight");
        if(module != NULL)
        {
            TLControlMode = tmodule->par("TLControlMode").longValue();
            activeDetection = tmodule->par("activeDetection").boolValue();
            minGreenTime = tmodule->par("minGreenTime").doubleValue();
        }
        else
        {
            TLControlMode = -1;
            activeDetection = false;
            minGreenTime = -1;
        }

        // simulate asynchronous channel access
        double offSet = dblrand() * (beaconInterval/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        if (sendBeacons)
        {
            RSUBeaconEvt = new cMessage("RSUBeaconEvt", KIND_TIMER);
            scheduleAt(simTime() + offSet, RSUBeaconEvt);
        }
	}
}


void ApplRSUBase::finish()
{
    BaseApplLayer::finish();

    // unsubscribe
    simulation.getSystemModule()->unsubscribe("executeEachTS", this);
}


void ApplRSUBase::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeEachTS)
    {
        executeEachTimeStep();
    }
}


void ApplRSUBase::handleSelfMsg(cMessage* msg)
{
    if (msg == RSUBeaconEvt)
    {
        BeaconRSU* beaconMsg = prepareBeacon();

        EV << "## Created beacon msg for " << myFullId << endl;

        // send it
        sendDelayed(beaconMsg, individualOffset, lowerLayerOut);

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, RSUBeaconEvt);
    }
}


void ApplRSUBase::executeEachTimeStep()
{

}


BeaconRSU* ApplRSUBase::prepareBeacon()
{
    BeaconRSU* wsm = new BeaconRSU("beaconRSU");

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
    wsm->setSender(myFullId);
    wsm->setSenderType("notSpecified");
    wsm->setRecipient("broadcast");

    // set my current position
    Coord SUMOpos (myCoordX, myCoordY);
    wsm->setPos(SUMOpos);

    return wsm;
}


}


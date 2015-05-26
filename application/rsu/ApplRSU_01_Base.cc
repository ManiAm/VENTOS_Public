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
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

		myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(getParentModule());
		assert(myMac);

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

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

        // simulate asynchronous channel access
        double offSet = dblrand() * (beaconInterval/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        RSUBeaconEvt = new cMessage("RSUBeaconEvt", KIND_TIMER);
        if (sendBeacons)
        {
            scheduleAt(simTime() + offSet, RSUBeaconEvt);
        }
	}
}


void ApplRSUBase::finish()
{
    BaseApplLayer::finish();
}


void ApplRSUBase::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeEachTS)
    {
        executeEachTimeStep((bool)i);
    }
}


void ApplRSUBase::handleSelfMsg(cMessage* msg)
{
    if (msg == RSUBeaconEvt)
    {
        BeaconRSU* beaconMsg = prepareBeacon();

        EV << "## Created beacon msg for " << myFullId << endl;
        printBeaconContent(beaconMsg);

        // send it
        sendDelayed(beaconMsg, individualOffset, lowerLayerOut);

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, RSUBeaconEvt);
    }
}


void ApplRSUBase::executeEachTimeStep(bool simulationDone)
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

    wsm->setChannelNumber(Channels::CCH);

    wsm->setDataRate(1);
    wsm->setPriority(beaconPriority);
    wsm->setPsid(0);

    // wsm->setSerial(serial);
    // wsm->setTimestamp(simTime());

    // fill in the sender/receiver fields
    wsm->setSender(myFullId);
    wsm->setRecipient("broadcast");

    // set my current position
    Coord *SUMOpos = getRSUsCoord(myId);
    wsm->setPos(*SUMOpos);

    return wsm;
}


// print beacon fields (for debugging purposes)
void ApplRSUBase::printBeaconContent(BeaconRSU* wsm)
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
    EV << wsm->getPos() << endl;
}


// todo
Coord *ApplRSUBase::getRSUsCoord(unsigned int index)
{
//    if( RSUs.size() == 0 )
//        error("No RSUs have been initialized!");
//
//    if( index < 0 || index >= RSUs.size() )
//        error("index out of bound!");
//
//    Coord *point = new Coord(RSUs[index]->coordX, RSUs[index]->coordY);
//    return point;

    Coord *point = new Coord(0, 0);
    return point;
}

}


/****************************************************************************/
/// @file    ApplAdversary.h
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

#include "ApplAdversary.h"

namespace VENTOS {

const simsignalwrap_t ApplAdversary::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

Define_Module(VENTOS::ApplAdversary);

ApplAdversary::~ApplAdversary()
{

}


void ApplAdversary::initialize(int stage)
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

        nodePtr->subscribe(mobilityStateChangedSignal, this);

        // vehicle id in omnet++
		myId = getParentModule()->getIndex();

		myFullId = getParentModule()->getFullName();

		AttackT = par("AttackT").doubleValue();
		falsificationAttack = par("falsificationAttack").boolValue();
		replayAttack = par("replayAttack").boolValue();
		jammingAttack = par("jammingAttack").boolValue();

        JammingEvt = new cMessage("Jamming Event");

        if(jammingAttack)
	    {
            //scheduleAt(simTime(), JammingEvt);
	    }
	}
}


void ApplAdversary::finish()
{
    nodePtr->unsubscribe(mobilityStateChangedSignal, this);
}


void ApplAdversary::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj)
{
    Enter_Method_Silent();

    if (signalID == mobilityStateChangedSignal)
    {
        ApplAdversary::handlePositionUpdate(obj);
    }
}


void ApplAdversary::handleSelfMsg(cMessage* msg)
{
    if(msg == JammingEvt)
    {
        if(simTime().dbl() >= AttackT)
        {
            DoJammingAttack();
        }

        // schedule for next jamming attack
        scheduleAt(simTime() + 0.001, JammingEvt);
    }
}


void ApplAdversary::handleLowerMsg(cMessage* msg)
{
    // Attack time has not arrived yet!
    if(simTime().dbl() < AttackT)
        return;

    // make sure msg is of type WaveShortMessage
    Veins::WaveShortMessage* wsm = dynamic_cast<Veins::WaveShortMessage*>(msg);
    ASSERT(wsm);

    if ( std::string(wsm->getName()) == "beaconVehicle" )
    {
        BeaconVehicle* wsm = dynamic_cast<BeaconVehicle*>(msg);
        ASSERT(wsm);

        EV << "######### received a beacon!" << endl;

        if(falsificationAttack)
        {
            DoFalsificationAttack(wsm);
        }
        else if(replayAttack)
        {
            DoReplayAttack(wsm);
        }
    }

    delete msg;
}


void ApplAdversary::handlePositionUpdate(cObject* obj)
{
    ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
}


// adversary get a msg, modifies the acceleration and re-send it
void ApplAdversary::DoFalsificationAttack(BeaconVehicle* wsm)
{
    // duplicate the received beacon
    BeaconVehicle* FalseMsg = wsm->dup();

    // alter the acceleration field
    FalseMsg->setAccel(6.);

    // alter the position field
    //Coord *newCord = new Coord(0,0);
    //FalseMsg->setPos(*newCord);

    // send it
    sendDelayed(FalseMsg, 0., lowerLayerOut);

    EV << "## Altered msg is sent." << endl;
}


// adversary get a msg and re-send it with a delay (without altering the content)
void ApplAdversary::DoReplayAttack(BeaconVehicle * wsm)
{
    // duplicate the received beacon
    BeaconVehicle* FalseMsg = wsm->dup();

    // send it with delay
    double delay = 10;
    sendDelayed(FalseMsg, delay, lowerLayerOut);

    EV << "## Altered msg is sent with delay of " << delay << endl;
}


void ApplAdversary::DoJammingAttack()
{
    DummyMsg* dm = CreateDummyMessage();

    // send it
    sendDelayed(dm, 0, lowerLayerOut);
}


DummyMsg* ApplAdversary::CreateDummyMessage()
{
    DummyMsg* wsm = new DummyMsg("dummy");

    wsm->addBitLength(10000);

    wsm->setWsmVersion(1);
    wsm->setSecurityType(1);

    wsm->setChannelNumber(Veins::Channels::CCH);

    wsm->setDataRate(1);
    wsm->setPriority(2);
    wsm->setPsid(0);

    wsm->setPayload("dummy");

    return wsm;
}

}

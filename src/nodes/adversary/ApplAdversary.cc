/****************************************************************************/
/// @file    ApplAdversary.cc
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

#include <nodes/adversary/ApplAdversary.h>

namespace VENTOS {

Define_Module(VENTOS::ApplAdversary);

ApplAdversary::~ApplAdversary()
{

}


void ApplAdversary::initialize(int stage)
{
	super::initialize(stage);

	if (stage==0)
	{
		AttackT = par("AttackT").doubleValue();
		falsificationAttack = par("falsificationAttack").boolValue();
		replayAttack = par("replayAttack").boolValue();
		jammingAttack = par("jammingAttack").boolValue();

        JammingEvt = new omnetpp::cMessage("Jamming Event");

        if(jammingAttack)
	    {
            //scheduleAt(simTime(), JammingEvt);
	    }
	}
}


void ApplAdversary::finish()
{
    super::finish();
}


void ApplAdversary::handleSelfMsg(omnetpp::cMessage* msg)
{
    if(msg == JammingEvt)
    {
        if(omnetpp::simTime().dbl() >= AttackT)
            DoJammingAttack();

        // schedule for next jamming attack
        scheduleAt(omnetpp::simTime() + 0.001, JammingEvt);
    }
    else
        super::handleSelfMsg(msg);
}


void ApplAdversary::handleLowerMsg(omnetpp::cMessage* msg)
{
    super::handleLowerMsg(msg);

    // Attack time has not arrived yet!
    if(omnetpp::simTime().dbl() < AttackT)
        return;

    if (msg->getKind() == TYPE_BEACON_VEHICLE)
    {
        BeaconVehicle* wsm = dynamic_cast<BeaconVehicle*>(msg);
        ASSERT(wsm);

        EV << "######### received a beacon!" << std::endl;

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
    send(FalseMsg, lowerLayerOut);

    EV << "## Altered msg is sent." << std::endl;
}


// adversary get a msg and re-send it with a delay (without altering the content)
void ApplAdversary::DoReplayAttack(BeaconVehicle * wsm)
{
    // duplicate the received beacon
    BeaconVehicle* FalseMsg = wsm->dup();

    // send it with delay
    double delay = 10;
    sendDelayed(FalseMsg, delay, lowerLayerOut);

    EV << "## Altered msg is sent with delay of " << delay << std::endl;
}


void ApplAdversary::DoJammingAttack()
{
    DummyMsg* dm = CreateDummyMessage();

    // send it
    send(dm, lowerLayerOut);
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

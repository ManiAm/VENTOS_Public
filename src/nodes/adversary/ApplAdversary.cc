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
#include "nodes/vehicle/Manager.h"

namespace VENTOS {

Define_Module(VENTOS::ApplAdversary);

ApplAdversary::~ApplAdversary()
{

}


void ApplAdversary::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        attackMode = par("attackMode").longValue();

        if(attackMode == -1)
            return;

        if(attackMode >= attackMode_MAX)
            throw omnetpp::cRuntimeError("Attack mode '%d' is not valid", attackMode);

        attackStartTime = par("attackStartTime").doubleValue();

        // get a pointer to the connection manager module
        omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("connMan");
        cc = static_cast<BaseConnectionManager*>(module);
        ASSERT(cc);

        Signal_executeEachTS = registerSignal("executeEachTimeStepSignal");
        omnetpp::getSimulation()->getSystemModule()->subscribe("executeEachTimeStepSignal", this);
    }
}


void ApplAdversary::finish()
{
    super::finish();

    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("executeEachTimeStepSignal", this);
}


void ApplAdversary::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeEachTS)
    {
        if(attackMode == attackMode_jamming)
        {
            if(omnetpp::simTime().dbl() >= attackStartTime)
                DoJammingAttack();
        }
    }
}


void ApplAdversary::handleSelfMsg(omnetpp::cMessage* msg)
{
    super::handleSelfMsg(msg);
}


void ApplAdversary::handleLowerMsg(omnetpp::cMessage* msg)
{
    // Only DSRC-enabled adversary accept this msg
    if(!DSRCenabled)
    {
        delete msg;
        return;
    }

    // Attack time has not arrived yet!
    if(omnetpp::simTime().dbl() < attackStartTime)
        return;

    // received a beacon from a nearby vehicle
    if (msg->getKind() == TYPE_BEACON_VEHICLE)
    {
        BeaconVehicle* wsm = dynamic_cast<BeaconVehicle*>(msg);
        ASSERT(wsm);

        if(attackMode == attackMode_falsification)
            DoFalsificationAttack(wsm);
        else if(attackMode == attackMode_replay)
            DoReplayAttack(wsm);
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
    // all NIC gates that are in the radio range of the adversary
    auto gateList = cc->getGateList(getParentModule()->getSubmodule("nic")->getId());
    // save in-range vehicle's name
    std::vector<std::string> inRangeVehs;
    for(auto &i : gateList)
    {
        std::string vehId = i.second->getOwnerModule()->getFullName();
        inRangeVehs.push_back(vehId);
    }

    static std::set<std::string> subscribedVehicles;

    // check for vehicles that need subscribing to
    std::set<std::string> needSubscribe;
    std::set_difference(inRangeVehs.begin(), inRangeVehs.end(), subscribedVehicles.begin(), subscribedVehicles.end(), std::inserter(needSubscribe, needSubscribe.begin()));
    for (auto i = needSubscribe.begin(); i != needSubscribe.end(); ++i)
    {
        subscribedVehicles.insert(*i);

        // get a pointer to the vehicle module
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getModuleByPath((*i).c_str());
        ASSERT(module);

        cModule *appl = module->getSubmodule("appl");
        ASSERT(appl);

        // get a pointer to the application layer
        ApplVManager *vehPtr = static_cast<ApplVManager *>(appl);
        ASSERT(vehPtr);

        // set jamming flag
        vehPtr->jamming = true;
    }

    // check for vehicles that need unsubscribing from
    std::set<std::string> needUnsubscribe;
    std::set_difference(subscribedVehicles.begin(), subscribedVehicles.end(), inRangeVehs.begin(), inRangeVehs.end(), std::inserter(needUnsubscribe, needUnsubscribe.begin()));
    for (auto i = needUnsubscribe.begin(); i != needUnsubscribe.end(); ++i)
    {
        subscribedVehicles.erase(*i);

        // get a pointer to the vehicle module
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getModuleByPath((*i).c_str());
        ASSERT(module);

        cModule *appl = module->getSubmodule("appl");
        ASSERT(appl);

        // get a pointer to the application layer
        ApplVManager *vehPtr = static_cast<ApplVManager *>(appl);
        ASSERT(vehPtr);

        // un-set jamming flag
        vehPtr->jamming = false;
    }

    //    DummyMsg* dm = CreateDummyMessage();
    //
    //    // send it
    //    send(dm, lowerLayerOut);
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

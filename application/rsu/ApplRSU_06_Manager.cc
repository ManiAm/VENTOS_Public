/****************************************************************************/
/// @file    ApplRSU_06_Manager.cc
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

#include "ApplRSU_06_Manager.h"
#include "SignalObj.h"

namespace VENTOS {

Define_Module(VENTOS::ApplRSUManager);

ApplRSUManager::~ApplRSUManager()
{

}


void ApplRSUManager::initialize(int stage)
{
    ApplRSUCRL::initialize(stage);

	if (stage==0)
	{
        // register signal
        Signal_executeEachTS = registerSignal("executeEachTS");
        simulation.getSystemModule()->subscribe("executeEachTS", this);

        Signal_beaconSignaling = registerSignal("beaconSignaling");
        simulation.getSystemModule()->subscribe("beaconSignaling", this);

        Signal_CRL_pieces = registerSignal("CRL_pieces");
        simulation.getSystemModule()->subscribe("CRL_pieces", this);
	}
}


void ApplRSUManager::finish()
{
    ApplRSUCRL::finish();

    // unsubscribe
    simulation.getSystemModule()->unsubscribe("beaconSignaling", this);
    simulation.getSystemModule()->unsubscribe("CRL_pieces", this);
}


void ApplRSUManager::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeEachTS)
    {
        executeEachTimeStep();
    }
}


void ApplRSUManager::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method_Silent();

    if(signalID == Signal_beaconSignaling)
    {
        cMessage* msg = static_cast<cMessage *>(obj);
        if (msg == NULL) return;

        // treat this as a normal msg
        // calling ApplRSU_04_Manager::handleLowerMsg
        handleLowerMsg(msg);
    }
    // CA sends this RSU a CRL
    else if(signalID == Signal_CRL_pieces)
    {
        CRLPiecesData *m = static_cast<CRLPiecesData *>(obj);
        ASSERT(m);

        if(m->name == std::string(myFullId))
            recieveCRL(m->data);
    }
}


void ApplRSUManager::handleSelfMsg(cMessage* msg)
{
    ApplRSUCRL::handleSelfMsg(msg);

}


void ApplRSUManager::handleLowerMsg(cMessage* msg)
{
    // make sure msg is of type WaveShortMessage
    Veins::WaveShortMessage* wsm = dynamic_cast<Veins::WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()) == "beaconVehicle")
    {
        BeaconVehicle* wsm = dynamic_cast<BeaconVehicle*>(msg);
        ASSERT(wsm);

        ApplRSUManager::onBeaconVehicle(wsm);
    }
    else if (std::string(wsm->getName()) == "beaconBicycle")
    {
        BeaconBicycle* wsm = dynamic_cast<BeaconBicycle*>(msg);
        ASSERT(wsm);

        ApplRSUManager::onBeaconBicycle(wsm);
    }
    else if (std::string(wsm->getName()) == "beaconPedestrian")
    {
        BeaconPedestrian* wsm = dynamic_cast<BeaconPedestrian*>(msg);
        ASSERT(wsm);

        ApplRSUManager::onBeaconPedestrian(wsm);
    }
    else if (std::string(wsm->getName()) == "beaconRSU")
    {
        BeaconRSU* wsm = dynamic_cast<BeaconRSU*>(msg);
        ASSERT(wsm);

        ApplRSUManager::onBeaconRSU(wsm);
    }
    else if(std::string(wsm->getName()) == "laneChangeMsg")
    {
        LaneChangeMsg* wsm = dynamic_cast<LaneChangeMsg*>(msg);
        ASSERT(wsm);

        ApplRSUManager::onData(wsm);
    }

    delete msg;
}


void ApplRSUManager::executeEachTimeStep()
{
    ApplRSUCRL::executeEachTimeStep();
}


void ApplRSUManager::onBeaconVehicle(BeaconVehicle* wsm)
{
    ApplRSUCRL::onBeaconVehicle(wsm);
}


void ApplRSUManager::onBeaconBicycle(BeaconBicycle* wsm)
{
    ApplRSUCRL::onBeaconBicycle(wsm);
}


void ApplRSUManager::onBeaconPedestrian(BeaconPedestrian* wsm)
{
    ApplRSUCRL::onBeaconPedestrian(wsm);
}


void ApplRSUManager::onBeaconRSU(BeaconRSU* wsm)
{
    ApplRSUCRL::onBeaconRSU(wsm);
}


void ApplRSUManager::onData(LaneChangeMsg* wsm)
{
    ApplRSUCRL::onData(wsm);
}

}

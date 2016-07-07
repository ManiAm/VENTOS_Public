/****************************************************************************/
/// @file    TL_Manager.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
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

#include "08_TL_Manager.h"

namespace VENTOS {

Define_Module(VENTOS::TrafficLightManager);


TrafficLightManager::~TrafficLightManager()
{

}


void TrafficLightManager::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initialize_withTraCI", this);

        Signal_executeEachTS = registerSignal("executeEachTS");
        omnetpp::getSimulation()->getSystemModule()->subscribe("executeEachTS", this);
    }
}


void TrafficLightManager::finish()
{
    super::finish();

    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("initialize_withTraCI", this);
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("executeEachTS", this);
}


void TrafficLightManager::handleMessage(omnetpp::cMessage *msg)
{
    super::handleMessage(msg);

}


void TrafficLightManager::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        initialize_withTraCI();
    }
    else if(signalID == Signal_executeEachTS)
    {
        executeEachTimeStep();
    }
}


void TrafficLightManager::initialize_withTraCI()
{
    // call parent
    super::initialize_withTraCI();

    // check if the TLControlMode number is valid?
    if(TLControlMode < 0 || TLControlMode > 10)
    {
        throw omnetpp::cRuntimeError("Invalid TLControlMode!");
    }
}


void TrafficLightManager::executeEachTimeStep()
{
    // call parent
    super::executeEachTimeStep();
}

}

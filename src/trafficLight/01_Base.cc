/****************************************************************************/
/// @file    Base.cc
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

#include "trafficLight/01_Base.h"

namespace VENTOS {

Define_Module(VENTOS::TrafficLightBase);


TrafficLightBase::~TrafficLightBase()
{

}


void TrafficLightBase::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
	{
        // get a pointer to the TraCI module
        TraCI = TraCI_Commands::getTraCI();

        TLControlMode = par("TLControlMode").intValue();

        // check if the TLControlMode number is valid?
        if(TLControlMode < 0 || TLControlMode >= NUM_TL_CONTROLS)
            throw omnetpp::cRuntimeError("Invalid TLControlMode!");

        debugLevel = omnetpp::getSimulation()->getSystemModule()->par("debugLevel").intValue();
    }
}


void TrafficLightBase::finish()
{

}


void TrafficLightBase::handleMessage(omnetpp::cMessage *msg)
{
    throw omnetpp::cRuntimeError("Can't handle msg '%s' of kind '%d'", msg->getFullName(), msg->getKind());
}


void TrafficLightBase::initialize_withTraCI()
{
    updateInterval = (double)TraCI->simulationGetDelta() / 1000.;
}


void TrafficLightBase::executeEachTimeStep()
{

}


ApplRSUMonitor * TrafficLightBase::findRSU(std::string TLid)
{
    // get a pointer to the first RSU
    cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("RSU", 0);
    if(module == NULL)
        throw omnetpp::cRuntimeError("No RSU module was found in the network!");

    // how many RSUs are in the network?
    int RSUcount = module->getVectorSize();

    // iterate over RSUs
    bool found = false;
    ApplRSUMonitor *RSUptr = NULL;
    for(int i = 0; i < RSUcount; ++i)
    {
        module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("RSU", i);
        std::string myTLid = module->par("myTLid").stringValue();

        // we found our RSU
        if(myTLid == TLid)
        {
            cModule *appl =  module->getSubmodule("appl");
            RSUptr = static_cast<ApplRSUMonitor *>(appl);
            if(RSUptr == NULL)
                throw omnetpp::cRuntimeError("Can not get a reference to our RSU!");

            found = true;
            break;
        }
    }

    if(!found)
        throw omnetpp::cRuntimeError("TL %s does not have any RSU!", TLid.c_str());

    return RSUptr;
}

}

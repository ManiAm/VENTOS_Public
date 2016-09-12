/****************************************************************************/
/// @file    tutorial.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Sep 2016
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

#include "tutorial.h"

namespace VENTOS {

// This macro registers this class with OMNET++
Define_Module(VENTOS::tutorial);

tutorial::~tutorial()
{

}

void tutorial::initialize(int stage)
{
    if(stage == 0)
    {
        // get a pointer to the TraCI module
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        // make sure module TraCI exists
        ASSERT(module);
        // get a pointer to TraCI class
        TraCI = static_cast<TraCI_Commands *>(module);
        // make sure the TraCI pointer is not null
        ASSERT(TraCI);

        // register and subscribe to Signal_initialize_withTraCI
        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initialize_withTraCI", this);

        // register and subscribe to Signal_executeEachTS
        Signal_executeEachTS = registerSignal("executeEachTS");
        omnetpp::getSimulation()->getSystemModule()->subscribe("executeEachTS", this);
    }
}

void tutorial::finish()
{
    // unsubscribe from both signals
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("initialize_withTraCI", this);
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("executeEachTS", this);
}

void tutorial::handleMessage(omnetpp::cMessage *msg)
{

}

void tutorial::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    // if we receive Signal_executeEachTS then call method executeEachTimestep()
    if(signalID == Signal_executeEachTS)
    {
        tutorial::executeEachTimestep();
    }
    // if we receive Signal_initialize_withTraCI then call method initialize_withTraCI()
    else if(signalID == Signal_initialize_withTraCI)
    {
        tutorial::initialize_withTraCI();
    }
}

void tutorial::initialize_withTraCI()
{

}

void tutorial::executeEachTimestep()
{

}

}

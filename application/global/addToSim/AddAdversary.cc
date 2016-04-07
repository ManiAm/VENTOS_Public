/****************************************************************************/
/// @file    AddAdversary.cc
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

#include "AddAdversary.h"

namespace VENTOS {

Define_Module(VENTOS::AddAdversary);


AddAdversary::~AddAdversary()
{

}


void AddAdversary::initialize(int stage)
{
    if(stage ==0)
    {
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        Signal_executeFirstTS = registerSignal("executeFirstTS");
        simulation.getSystemModule()->subscribe("executeFirstTS", this);

        on = par("on").boolValue();
        mode = par("mode").longValue();
    }
}


void AddAdversary::finish()
{

}


void AddAdversary::handleMessage(cMessage *msg)
{

}


void AddAdversary::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeFirstTS)
    {
        AddAdversary::Add();
    }
}


void AddAdversary::Add()
{
    // if dynamic adding is off, return
    if (!on)
        return;

    cModule* parentMod = getParentModule();
    if (!parentMod) error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get("c3po.application.adversary.Adversary");

    // do not use create("adversary", parentMod);
    // instead create an array of adversaries
    cModule* mod = nodeType->create("adversary", parentMod, 1, 0);
    mod->finalizeParameters();
    mod->getDisplayString().updateWith("i=old/comp_a");
    mod->buildInside();
    mod->scheduleStart(simTime());
    mod->callInitialize();
}

}


/* -*- mode:c++ -*- ********************************************************
 * file:        BaseModule.cc
 *
 * author:      Steffen Sroka
 *              Andreas Koepke
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/

#include <baseAppl/01_BaseModule.h>
#include <cassert>

#include "global/FindModule.h"

// Could not initialize simsignal_t it here!? I got the POST_MODEL_CHANGE id!?
const simsignalwrap_t BaseModule::catHostStateSignal = simsignalwrap_t(MIXIM_SIGNAL_HOSTSTATE_NAME);

BaseModule::BaseModule(): cSimpleModule()
{

}


BaseModule::BaseModule(unsigned stacksize): cSimpleModule(stacksize)
{

}

/**
 * Subscription should be in stage==0, and firing
 * notifications in stage==1 or later.
 *
 * NOTE: You have to call this in the initialize() function of the
 * inherited class!
 */
void BaseModule::initialize(int stage)
{
    if (stage == 0)
    {
        notAffectedByHostState = hasPar("notAffectedByHostState") && par("notAffectedByHostState").boolValue();
        hasPar("debug") ? debug = par("debug").boolValue() : debug = true;
        findHost()->subscribe(catHostStateSignal, this);
    }
}


void BaseModule::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, omnetpp::cObject *obj, cObject* details)
{
    Enter_Method_Silent();

    if (signalID == catHostStateSignal)
    {
        const HostState *const pHostState = dynamic_cast<const HostState *const>(obj);
        if (pHostState)
            handleHostState(*pHostState);
        else
            throw omnetpp::cRuntimeError("Got catHostStateSignal but obj was not a HostState pointer?");
    }
}


void BaseModule::handleHostState(const HostState& state)
{
    if(notAffectedByHostState)
        return;

    if(state.get() != HostState::ACTIVE)
    {
        throw omnetpp::cRuntimeError("Hosts state changed to something else than active which"
                " is not handled by this module. Either handle this state"
                " correctly or if this module really isn't affected by the"
                " hosts state set the parameter \"notAffectedByHostState\""
                " of this module to true.");
    }
}


void BaseModule::switchHostState(HostState::States state)
{
    HostState hostState(state);
    emit(catHostStateSignal, &hostState);
}


omnetpp::cModule *const BaseModule::findHost(void)
{
    return FindModule<>::findHost(this);
}


const omnetpp::cModule *const BaseModule::findHost(void) const
{
    return FindModule<>::findHost(this);
}


std::string BaseModule::logName(void) const
{
    std::ostringstream ost;
    if (hasPar("logName")) // let modules override
    {
        ost << par("logName").stringValue();
    }
    else
    {
        const cModule *const parent = findHost();
        parent->hasPar("logName") ? ost << parent->par("logName").stringValue() : ost << parent->getName();
        ost << "[" << parent->getIndex() << "]";
    }

    return ost.str();
}

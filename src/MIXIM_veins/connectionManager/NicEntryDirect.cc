/* -*- mode:c++ -*- ********************************************************
 * file:        NicEntryDirect.cc
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2005 Telecommunication Networks Group (TKN) at
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
 * description: Class to store information about a nic for the
 *              ConnectionManager module
 **************************************************************************/


#include "NicEntryDirect.h"
#include "MIXIM_veins/nic/phy/ChannelAccess.h"

#ifndef nicEV
#define nicEV EV << "NicEntry: "
#endif


void NicEntryDirect::connectTo(NicEntry* other)
{
    omnetpp::cModule* otherPtr = other->nicPtr;

    nicEV << "connecting nic #" << nicId << " and #" << other->nicId << std::endl;

    omnetpp::cGate *radioGate=NULL;
    if( (radioGate = otherPtr->gate("radioIn")) == NULL )
		throw omnetpp::cRuntimeError("Nic has no radioIn gate!");

    outConns[other] = radioGate->getPathStartGate();
}


void NicEntryDirect::disconnectFrom(NicEntry* other)
{
    nicEV << "disconnecting nic #" << nicId << " and #" << other->nicId << std::endl;
    outConns.erase(other);
}

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

#include "vehicle/01_Base.h"

namespace VENTOS {

const simsignalwrap_t ApplVBase::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

Define_Module(VENTOS::ApplVBase);

ApplVBase::~ApplVBase()
{

}

void ApplVBase::initialize(int stage)
{
    super::initialize(stage);

    if (stage==0)
    {
        // get a pointer to the TraCI module
        omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        // get a pointer to the Statistics module
        module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("statistics");
        STAT = static_cast<VENTOS::Statistics *>(module);
        ASSERT(STAT);

        headerLength = par("headerLength").longValue();

        // vehicle id in omnet++
        myId = getParentModule()->getIndex();
        // vehicle full id in omnet++
        myFullId = getParentModule()->getFullName();
        // vehicle id in sumo
        SUMOID = getParentModule()->par("SUMOID").stringValue();
        // vehicle type in sumo
        SUMOType = getParentModule()->par("SUMOType").stringValue();
        // vehicle class in sumo
        vehicleClass = getParentModule()->par("vehicleClass").stringValue();

        hasOBU = getParentModule()->par("hasOBU").boolValue();
        IPaddress = getParentModule()->par("IPaddress").stringValue();

        // get controller type from SUMO
        SUMOControllerType = getParentModule()->par("SUMOControllerType").longValue();
        // get controller number from SUMO
        SUMOControllerNumber = getParentModule()->par("SUMOControllerNumber").longValue();

        // store the time of entry
        entryTime = omnetpp::simTime().dbl();
    }
}


void ApplVBase::finish()
{

}


void ApplVBase::handleSelfMsg(omnetpp::cMessage* msg)
{
    throw omnetpp::cRuntimeError("Can't handle msg %s of kind %d", msg->getFullName(), msg->getKind());
}


// is called, every time the position of vehicle changes
void ApplVBase::handlePositionUpdate(cObject* obj)
{

}

}


/****************************************************************************/
/// @file    ApplPed_01_Base.cc
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

#include "ApplPed_01_Base.h"

namespace VENTOS {

Define_Module(VENTOS::ApplPedBase);

ApplPedBase::~ApplPedBase()
{

}

void ApplPedBase::initialize(int stage)
{
	super::initialize(stage);

	if (stage==0)
	{
        // get a pointer to the TraCI module
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        // get a pointer to the traffic light
        module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TrafficLight");
        TLControlMode = module->par("TLControlMode").longValue();

        headerLength = par("headerLength").longValue();

        // pedestrian id in omnet++
		myId = getParentModule()->getIndex();
		// pedestrian full id in omnet++
		myFullId = getParentModule()->getFullName();
        // pedestrian id in sumo
        SUMOID = par("SUMOID").stringValue();
        // pedestrian type in sumo
        SUMOType = par("SUMOType").stringValue();
        // vehicle class in sumo
        vehicleClass = par("vehicleClass").stringValue();
        // vehicle class code
        vehicleClassEnum = par("vehicleClassEnum").longValue();

        // store the time of entry
        entryTime = omnetpp::simTime().dbl();
	}
}


void ApplPedBase::finish()
{

}


void ApplPedBase::handleSelfMsg(omnetpp::cMessage* msg)
{

}


// is called, every time the position of vehicle changes
void ApplPedBase::handlePositionUpdate(cObject* obj)
{

}

}


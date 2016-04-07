/****************************************************************************/
/// @file    ApplBike_01_Base.cc
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

#include "ApplBike_01_Base.h"

namespace VENTOS {

Define_Module(VENTOS::ApplBikeBase);

ApplBikeBase::~ApplBikeBase()
{

}

void ApplBikeBase::initialize(int stage)
{
	BaseApplLayer::initialize(stage);

	if (stage==0)
	{
        // get the ptr of the current module
        nodePtr = this->getParentModule();
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        // get a pointer to traffic light
        module = simulation.getSystemModule()->getSubmodule("TrafficLight");
        TLControlMode = module->par("TLControlMode").longValue();

        headerLength = par("headerLength").longValue();

        // bike id in omnet++
		myId = getParentModule()->getIndex();
        // bike full id in omnet++
		myFullId = getParentModule()->getFullName();
        // bike id in sumo
        SUMOID = par("SUMOID").stdstringValue();
        // bike type in sumo
        SUMOType = par("SUMOType").stdstringValue();
        // vehicle class in sumo
        vehicleClass = par("vehicleClass").stdstringValue();
        // vehicle class code
        vehicleClassEnum = par("vehicleClassEnum").longValue();

        // store the time of entry
        entryTime = simTime().dbl();
	}
}


void ApplBikeBase::finish()
{

}


void ApplBikeBase::handleSelfMsg(cMessage* msg)
{

}

}


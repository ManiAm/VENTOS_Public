/****************************************************************************/
/// @file    TraCIMobilityMod.cc
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

#include "TraCIMobilityMod.h"

namespace VENTOS {

Define_Module(VENTOS::TraCIMobilityMod);


void TraCIMobilityMod::initialize(int stage)
{
    TraCIMobility::initialize(stage);

    // get a pointer to the TraCI module
    cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
    if(module == NULL)
        throw("TraCI module not found!");

    moduleName = module->par("moduleName").stringValue();
    bikeModuleName = module->par("bikeModuleName").stringValue();
    pedModuleName = module->par("pedModuleName").stringValue();
}


void TraCIMobilityMod::updateDisplayString()
{
	ASSERT(-M_PI <= angle);
	ASSERT(angle < M_PI);

	// if this is a vehicle
	if(std::string(getParentModule()->getName()) == moduleName)
	{
	    getParentModule()->getDisplayString().setTagArg("b", 2, "rect");
	    getParentModule()->getDisplayString().setTagArg("b", 3, "red");
	    getParentModule()->getDisplayString().setTagArg("b", 4, "black");
	    getParentModule()->getDisplayString().setTagArg("b", 5, "1");
	}
    // if this is a bicycle
	else if(std::string(getParentModule()->getName()) == bikeModuleName)
    {
        getParentModule()->getDisplayString().setTagArg("b", 2, "rect");
        getParentModule()->getDisplayString().setTagArg("b", 3, "yellow");
        getParentModule()->getDisplayString().setTagArg("b", 4, "black");
        getParentModule()->getDisplayString().setTagArg("b", 5, "1");
    }
    // if this is a pedestrian
	else if(std::string(getParentModule()->getName()) == pedModuleName)
    {
        getParentModule()->getDisplayString().setTagArg("b", 2, "rect");
        getParentModule()->getDisplayString().setTagArg("b", 3, "green");
        getParentModule()->getDisplayString().setTagArg("b", 4, "black");
        getParentModule()->getDisplayString().setTagArg("b", 5, "1");
    }



	if (angle < -M_PI + 0.5 * M_PI_4 * 1) {
		getParentModule()->getDisplayString().setTagArg("t", 0, "\u2190");
		getParentModule()->getDisplayString().setTagArg("b", 0, "4");
		getParentModule()->getDisplayString().setTagArg("b", 1, "2");
	}
	else if (angle < -M_PI + 0.5 * M_PI_4 * 3) {
		getParentModule()->getDisplayString().setTagArg("t", 0, "\u2199");
		getParentModule()->getDisplayString().setTagArg("b", 0, "3");
		getParentModule()->getDisplayString().setTagArg("b", 1, "3");
	}
	else if (angle < -M_PI + 0.5 * M_PI_4 * 5) {
		getParentModule()->getDisplayString().setTagArg("t", 0, "\u2193");
		getParentModule()->getDisplayString().setTagArg("b", 0, "2");
		getParentModule()->getDisplayString().setTagArg("b", 1, "4");
	}
	else if (angle < -M_PI + 0.5 * M_PI_4 * 7) {
		getParentModule()->getDisplayString().setTagArg("t", 0, "\u2198");
		getParentModule()->getDisplayString().setTagArg("b", 0, "3");
		getParentModule()->getDisplayString().setTagArg("b", 1, "3");
	}
	else if (angle < -M_PI + 0.5 * M_PI_4 * 9) {
		getParentModule()->getDisplayString().setTagArg("t", 0, "\u2192");
		getParentModule()->getDisplayString().setTagArg("b", 0, "4");
		getParentModule()->getDisplayString().setTagArg("b", 1, "2");
	}
	else if (angle < -M_PI + 0.5 * M_PI_4 * 11) {
		getParentModule()->getDisplayString().setTagArg("t", 0, "\u2197");
		getParentModule()->getDisplayString().setTagArg("b", 0, "3");
		getParentModule()->getDisplayString().setTagArg("b", 1, "3");
	}
	else if (angle < -M_PI + 0.5 * M_PI_4 * 13) {
		getParentModule()->getDisplayString().setTagArg("t", 0, "\u2191");
		getParentModule()->getDisplayString().setTagArg("b", 0, "2");
		getParentModule()->getDisplayString().setTagArg("b", 1, "4");
	}
	else if (angle < -M_PI + 0.5 * M_PI_4 * 15) {
		getParentModule()->getDisplayString().setTagArg("t", 0, "\u2196");
		getParentModule()->getDisplayString().setTagArg("b", 0, "3");
		getParentModule()->getDisplayString().setTagArg("b", 1, "3");
	}
	else {
		getParentModule()->getDisplayString().setTagArg("t", 0, "\u2190");
		getParentModule()->getDisplayString().setTagArg("b", 0, "4");
		getParentModule()->getDisplayString().setTagArg("b", 1, "2");
	}
}

}




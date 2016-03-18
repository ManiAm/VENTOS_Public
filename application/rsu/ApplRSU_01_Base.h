/****************************************************************************/
/// @file    ApplRSU_01_Base.h
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

#ifndef APPLRSU_H_
#define APPLRSU_H_

#include <BaseApplLayer.h>
#include <WaveAppToMac1609_4Interface.h>
#include "TraCI_Commands.h"
#include "BeaconRSU_m.h"

namespace VENTOS {

class RSUAdd;

class ApplRSUBase : public BaseApplLayer
{
	public:
		~ApplRSUBase();
		virtual void initialize(int stage);
		virtual void finish();
        virtual void handleSelfMsg(cMessage* msg);

	protected:
        virtual void executeEachTimeStep();
        BeaconRSU* prepareBeacon();

	protected:
		// NED variables
	    cModule *nodePtr;   // pointer to the Node
	    TraCI_Commands* TraCI;
	    cModule* TLptr;

        // NED variables (beaconing parameters)
        bool sendBeacons;
        double beaconInterval;
        double maxOffset;
        int beaconLengthBits;
        int beaconPriority;

        // Class variables
        int myId;
		const char *myFullId;
        std::string myTLid;
        double myCoordX;    // my X coordinate in SUMO
        double myCoordY;    // my Y coordinate in SUMO

        simtime_t individualOffset;
        cMessage* RSUBeaconEvt;

        int TLControlMode = -1;
        double minGreenTime = -1;
};

}

#endif

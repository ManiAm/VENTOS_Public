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
#include "TraCI_Extend.h"

namespace VENTOS {

class TraCI_Extend;
class RSUAdd;

class ApplRSUBase : public BaseApplLayer
{
	public:
		~ApplRSUBase();
		virtual void initialize(int stage);
		virtual void finish();
        virtual void handleSelfMsg(cMessage* msg);
	    virtual void receiveSignal(cComponent *, simsignal_t, long);

		enum WaveApplMessageKinds
		{
			SERVICE_PROVIDER = LAST_BASE_APPL_MESSAGE_KIND,
			KIND_TIMER
		};

	protected:
        virtual void executeEachTimeStep(bool);
		Coord *getRSUsCoord(unsigned int);
        BeaconRSU* prepareBeacon();
        void printBeaconContent(BeaconRSU*);

	protected:
		// NED variables
	    cModule *nodePtr;   // pointer to the Node
        WaveAppToMac1609_4Interface* myMac;
        mutable TraCI_Extend* TraCI;

        // NED variables (beaconing parameters)
        bool sendBeacons;
        double beaconInterval;
        double maxOffset;
        int beaconLengthBits;
        int beaconPriority;

        // Class variables
        int myId;
		const char *myFullId;
        simtime_t individualOffset;
        cMessage* RSUBeaconEvt;

        std::string myTLid;

        static const simsignalwrap_t mobilityStateChangedSignal;
        simsignal_t Signal_executeEachTS;
};

}

#endif

/****************************************************************************/
/// @file    VehicleWarmup.h
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

#ifndef WARMPUP
#define WARMPUP

#include "TraCICommands.h"
#include "VehicleSpeedProfile.h"

namespace VENTOS {

class Warmup : public BaseApplLayer
{
	public:
		virtual ~Warmup();
		virtual void initialize(int stage);
        virtual void handleMessage(omnetpp::cMessage *msg);
		virtual void finish();
	    virtual void receiveSignal(omnetpp::cComponent *, omnetpp::simsignal_t, long, cObject* details);

	private:
        bool DoWarmup();

	private:
        // NED variables
        TraCI_Commands *TraCI;  // pointer to the TraCI module
        SpeedProfile *SpeedProfilePtr;

        // NED variables
        bool on;
        std::string laneId;
        double stopPosition;  // the position that first vehicle should stop waiting for others
        double warmUpSpeed;
        double waitingTime;
        int numVehicles;

        // class variables
        omnetpp::simsignal_t Signal_executeEachTS;
        double startTime;     // the time that Warmup starts
        bool IsWarmUpFinished;
        omnetpp::cMessage* finishingWarmup;
};

}

#endif

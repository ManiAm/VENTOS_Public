/****************************************************************************/
/// @file    Mac1609_4_Mod.cc
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

#include "Mac1609_4_Mod.h"
#include "SignalObj.h"

namespace VENTOS {

Define_Module(VENTOS::Mac1609_4_Mod);

void Mac1609_4_Mod::initialize(int stage)
{
    Mac1609_4::initialize(stage);

	if (stage == 0)
	{
        // get the ptr of the Statistics module
        cModule *module = simulation.getSystemModule()->getSubmodule("statistics");
        StatPtr = static_cast<Statistics *>(module);
        if(StatPtr == NULL)
            error("can not get a pointer to the Statistics module.");

        reportMAClayerData = StatPtr->par("reportMAClayerData").boolValue();
	}
}


void Mac1609_4_Mod::finish()
{
    Mac1609_4::finish();
}


void Mac1609_4_Mod::handleSelfMsg(cMessage* msg)
{
    Mac1609_4::handleSelfMsg(msg);
}


void Mac1609_4_Mod::handleUpperControl(cMessage* msg)
{
    Mac1609_4::handleUpperControl(msg);
}


// all messages from application layer are sent to this method!
void Mac1609_4_Mod::handleUpperMsg(cMessage* msg)
{
    Mac1609_4::handleUpperMsg(msg);

    if(!reportMAClayerData)
        return;

    // send signal to statistics
    std::vector<long> MacStats;

    MacStats.push_back(statsDroppedPackets); // packet was dropped in Mac
    MacStats.push_back(statsNumTooLittleTime); // Too little time in this interval. Will not schedule nextMacEvent
    MacStats.push_back(statsNumInternalContention); // there was already another packet ready.
                                                    // we have to go increase CW and go into backoff.
                                                    // It's called internal contention and its wonderful
    MacStats.push_back(statsNumBackoff);
    MacStats.push_back(statsSlotsBackoff);
    MacStats.push_back(statsTotalBusyTime.dbl());

    MacStats.push_back(statsSentPackets);

    MacStats.push_back(statsSNIRLostPackets);  // A packet was not received due to biterrors
    MacStats.push_back(statsTXRXLostPackets);  // A packet was not received because we were sending while receiving

    MacStats.push_back(statsReceivedPackets);  // Received a data packet addressed to me
    MacStats.push_back(statsReceivedBroadcasts);  // Received a broadcast data packet

    MacStat *vec = new MacStat(MacStats);

    simsignal_t Signal_MacStats = registerSignal("MacStats");
    this->emit(Signal_MacStats, vec);
}


void Mac1609_4_Mod::handleLowerControl(cMessage* msg)
{
    Mac1609_4::handleLowerControl(msg);
}


void Mac1609_4_Mod::handleLowerMsg(cMessage* msg)
{
    Mac1609_4::handleLowerMsg(msg);
}

}


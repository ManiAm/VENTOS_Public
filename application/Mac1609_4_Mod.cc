
#include "Mac1609_4_Mod.h"

namespace VENTOS {

Define_Module(VENTOS::Mac1609_4_Mod);

void Mac1609_4_Mod::initialize(int stage)
{
    Mac1609_4::initialize(stage);

	if (stage == 0)
	{
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");
	}
}


void Mac1609_4_Mod::handleSelfMsg(cMessage* msg)
{
    Mac1609_4::handleSelfMsg(msg);
}


void Mac1609_4_Mod::handleUpperControl(cMessage* msg)
{
    Mac1609_4::handleUpperControl(msg);
}


void Mac1609_4_Mod::handleUpperMsg(cMessage* msg)
{
    Mac1609_4::handleUpperMsg(msg);

    // send signal to statistics
    vector<long> MacStats;

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

//    MacStat *vec = new MacStat(MacStats);
//
//    simsignal_t Signal_MacStats = registerSignal("MacStats");
//    nodePtr->emit(Signal_MacStats, vec);
}


void Mac1609_4_Mod::handleLowerControl(cMessage* msg)
{
    Mac1609_4::handleLowerControl(msg);
}


void Mac1609_4_Mod::handleLowerMsg(cMessage* msg)
{
    Mac1609_4::handleLowerMsg(msg);
}


void Mac1609_4_Mod::finish()
{
    Mac1609_4::finish();

    //clean up queues.

    // todo: do not let record queue!
}
}


//
// Copyright (C) 2012 David Eckhoff <eckhoff@cs.fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
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

#ifndef ___MAC1609_4_ECDA_H_
#define ___MAC1609_4_ECDA_H_

#include <queue>
#include "msg/WaveShortMessage_m.h"
#include "MIXIM_veins/nic/Consts80211p.h"

namespace Veins {

// EDCA access categories in increasing order of priority (see IEEE Std 802.11-2012, Table 9-1)
enum t_access_category
{
    AC_BK = 0,  // background
    AC_BE = 1,  // best effort
    AC_VI = 2,  // video
    AC_VO = 3   // voice
};


class EDCA
{
public:
    typedef struct EDCAQueue
    {
        std::queue<WaveShortMessage*> queue;
        int aifsn;          // number of AIFS (arbitration inter-frame space) slots
        int cwMin;          // minimum contention window
        int cwMax;          // maximum contention size
        int cwCur;          // current contention window
        int64_t currentBackoff; // current backoff value
        bool txOP;          // Transmit Opportunity (TXOP)

        EDCAQueue() {   };
        EDCAQueue(int aifsn, int cwMin, int cwMax, t_access_category ac)
        {
            this->aifsn = aifsn;
            this->cwMin = cwMin;
            this->cwMax = cwMax;
            this->cwCur = cwMin;
            this->currentBackoff = 0;
            this->txOP = false;
        };
    } EDCAQueue_t;

    omnetpp::cModule *owner;
    std::map<t_access_category, EDCAQueue_t> myQueues;
    uint32_t maxQueueSize;
    omnetpp::simtime_t lastStart; //when we started the last contention;
    t_channel channelType;

    /** @brief Stats */
    long NumInternalContention;
    long NumBackoff;
    long SlotsBackoff;

    /** @brief Id for debug messages */
    std::string myId;

    EDCA(omnetpp::cModule *owner, t_channel channelType, int maxQueueLength = 0)
    {
        this->owner = owner;
        this->maxQueueSize = maxQueueLength;
        this->channelType = channelType;
        this->NumInternalContention = 0;
        this->NumBackoff = 0;
        this->SlotsBackoff = 0;
    };

    // @brief currently you have to call createQueue in the right order. First Call is priority 0, second 1 and so on...
    void createQueue(int aifsn, int cwMin, int cwMax, t_access_category);
    int queuePacket(t_access_category AC, WaveShortMessage* cmsg);
    void backoff(t_access_category ac);
    omnetpp::simtime_t startContent(omnetpp::simtime_t idleSince, bool guardActive);
    void stopContent(bool allowBackoff, bool generateTxOp);
    void postTransmit(t_access_category);
    void revokeTxOPs();
    void cleanUp();

    /** @brief return the next packet to send, send all lower Queues into backoff */
    WaveShortMessage* initiateTransmit(omnetpp::simtime_t idleSince);
};

}

#endif

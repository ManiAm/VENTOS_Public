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

#ifndef ___MAC1609_4_H_
#define ___MAC1609_4_H_

#include <assert.h>
#include <omnetpp.h>
#include <queue>
#include <stdint.h>

#include "baseAppl/02_BaseLayer.h"
#include "MacToPhyControlInfo.h"
#include "PhyLayer80211p.h"
#include "WaveAppToMac1609_4Interface.h"
#include "Consts80211p.h"
#include "FindModule.h"
#include "BaseMacLayer.h"
#include "ConstsPhy.h"

#include "Mac80211Pkt_m.h"
#include "WaveShortMessage_m.h"
#include "global/Statistics.h"

namespace Veins {

/**
 * @brief
 * Manages timeslots for CCH and SCH listening and sending.
 *
 * @author Mani Amoozadeh
 * @author David Eckhoff : rewrote complete model
 * @author Christoph Sommer : features and bug fixes
 * @author Michele Segata : features and bug fixes
 * @author Stefan Joerer : features and bug fixes
 * @author Christopher Saloman: initial version
 *
 * @ingroup macLayer
 *
 * @see BaseWaveApplLayer
 * @see Mac1609_4
 * @see PhyLayer80211p
 * @see Decider80211p
 */


// access categories for EDCA
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
        int currentBackoff; // current backoff value
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
    int numQueues;
    uint32_t maxQueueSize;
    omnetpp::simtime_t lastStart; //when we started the last contention;
    t_channel channelType;

    /** @brief Stats */
    long statsNumInternalContention;
    long statsNumBackoff;
    long statsSlotsBackoff;

    /** @brief Id for debug messages */
    std::string myId;

    EDCA(omnetpp::cModule *owner, t_channel channelType, int maxQueueLength = 0)
    {
        this->owner = owner;
        this->numQueues = 0;
        this->maxQueueSize = maxQueueLength;
        this->channelType = channelType;
        this->statsNumInternalContention = 0;
        this->statsNumBackoff = 0;
        this->statsSlotsBackoff = 0;
    };

    // @brief currently you have to call createQueue in the right order. First Call is priority 0, second 1 and so on...
    int createQueue(int aifsn, int cwMin, int cwMax, t_access_category);
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


class Mac1609_4 : public BaseMacLayer, public WaveAppToMac1609_4Interface
{
private:
    Mac80211pToPhy11pInterface* phy11p;
    VENTOS::Statistics* STAT;

    bool record_stat;

protected:
    /** @brief Self message to indicate that the current channel shall be switched.*/
    omnetpp::cMessage* nextChannelSwitch;

    /** @brief Self message to wake up at next MacEvent */
    omnetpp::cMessage* nextMacEvent;

    /** @brief Last time the channel went idle */
    omnetpp::simtime_t lastIdle;
    omnetpp::simtime_t lastBusy;

    /** @brief Current state of the channel selecting operation.*/
    t_channel activeChannel;

    /** @brief access category of last sent packet */
    t_access_category lastAC;

    /** @brief Stores the frequencies in Hz that are associated to the channel numbers.*/
    std::map<int, double> frequency;

    int headerLength;

    bool useSCH;
    int mySCH;

    std::map<t_channel, EDCA*> myEDCA;

    bool idleChannel = true;

    /** @brief stats */
    long statsReceivedPackets = 0;
    long statsReceivedBroadcasts = 0;
    long statsSentPackets = 0;
    long statsTXRXLostPackets = 0;
    long statsSNIRLostPackets = 0;
    long statsDroppedPackets = 0;
    long statsNumTooLittleTime = 0;
    long statsNumInternalContention = 0;
    long statsNumBackoff = 0;
    long statsSlotsBackoff = 0;
    omnetpp::simtime_t statsTotalBusyTime = 0;

    /** @brief This MAC layers MAC address.*/
    int myMacAddress;

    /** @brief The power (in mW) to transmit with.*/
    double txPower;

    /** @brief the bit rate at which we transmit */
    uint64_t bitrate;

    /** @brief N_DBPS, derived from bitrate, for frame length calculation */
    double n_dbps = 0;

    /** @brief Id for debug messages */
    std::string myId;

    //tell to anybody which is interested when the channel turns busy or idle
    omnetpp::simsignal_t sigChannelBusy;
    //tell to anybody which is interested when a collision occurred
    omnetpp::simsignal_t sigCollision;

public:
    ~Mac1609_4() { };

    // return true if alternate access is enabled
    bool isChannelSwitchingActive();

    omnetpp::simtime_t getSwitchingInterval();

    bool isCurrentChannelCCH();

    void changeServiceChannel(int channelNumber);

    /**
     * @brief Change the default tx power the NIC card is using
     *
     * @param txPower_mW the tx power to be set in mW
     */
    void setTxPower(double txPower_mW);

    /**
     * @brief Change the default MCS the NIC card is using
     *
     * @param mcs the default modulation and coding scheme
     * to use
     */
    void setMCS(enum PHY_MCS mcs);

    /**
     * @brief Change the phy layer carrier sense threshold.
     *
     * @param ccaThreshold_dBm the cca threshold in dBm
     */
    void setCCAThreshold(double ccaThreshold_dBm);

protected:
    /** @brief Initialization of the module and some variables.*/
    virtual void initialize(int);

    /** @brief Delete all dynamically allocated objects of the module.*/
    virtual void finish();

    /** @brief Handle messages from lower layer.*/
    virtual void handleLowerMsg(omnetpp::cMessage*);

    /** @brief Handle messages from upper layer.*/
    virtual void handleUpperMsg(omnetpp::cMessage*);

    /** @brief Handle control messages from upper layer.*/
    virtual void handleUpperControl(omnetpp::cMessage* msg);

    /** @brief Handle self messages such as timers.*/
    virtual void handleSelfMsg(omnetpp::cMessage*);

    /** @brief Handle control messages from lower layer.*/
    virtual void handleLowerControl(omnetpp::cMessage* msg);

    /** @brief Set a state for the channel selecting operation.*/
    void setActiveChannel(t_channel state);

    omnetpp::simtime_t timeLeftInSlot() const;
    omnetpp::simtime_t timeLeftTillGuardOver() const;

    bool guardActive() const;

    void attachSignal(Mac80211Pkt* mac, omnetpp::simtime_t startTime, double frequency, uint64_t datarate, double txPower_mW);
    Signal* createSignal(omnetpp::simtime_t start, omnetpp::simtime_t length, double power, uint64_t bitrate, double frequency);

    /** @brief maps a application layer priority (up) to an EDCA access category. */
    t_access_category mapPriority(int prio);

    void channelBusy();
    void channelBusySelf(bool generateTxOp);
    void channelIdle(bool afterSwitch = false);

    void setParametersForBitrate(uint64_t bitrate);

    omnetpp::simtime_t getFrameDuration(int payloadLengthBits, enum PHY_MCS mcs = MCS_DEFAULT) const;

private:
    void record_MAC_stat_func();
};

}

#endif /* ___MAC1609_4_H_*/

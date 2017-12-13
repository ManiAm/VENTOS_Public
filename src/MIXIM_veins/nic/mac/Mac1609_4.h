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
#include <stdint.h>

#include "baseAppl/02_BaseLayer.h"
#include "MacToPhyControlInfo.h"
#include "MIXIM_veins/nic/phy/PhyLayer80211p.h"
#include "WaveAppToMac1609_4Interface.h"
#include "global/FindModule.h"
#include "ConstsPhy.h"
#include "Mac1609_4_EDCA.h"
#include "msg/Mac80211Pkt_m.h"
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

class Mac1609_4 : public BaseLayer, public WaveAppToMac1609_4Interface
{
public:

    /** @brief Message kinds used by this layer.*/
    enum BaseMacMessageKinds
    {
        /** Stores the id on which classes extending BaseMac should
         * continue their own message kinds.*/
        LAST_BASE_MAC_MESSAGE_KIND = 23000,
    };

    /** @brief Control message kinds used by this layer.*/
    enum BaseMacControlKinds
    {
        /** Indicates the end of a transmission*/
        TX_OVER = 23500,

        /** Tells the upper layer that a packet to be sent has been dropped.*/
        DROPPED,

        /** Stores the id on which classes extending BaseMac should
         * continue their own control kinds.*/
        LAST_BASE_MAC_CONTROL_KIND,
    };

private:

    /**
     * @brief Length of the MacPkt header
     **/
    int headerLength;

    /** @brief debug this core module? */
    bool coreDebug;

    /** @brief This MAC layers MAC address.*/
    LAddress::L2Type myMacAddress;

    /** @brief Id for debug messages */
    std::string myId;

    double txPower;
    uint64_t bitrate;

    bool record_stat;

    // reference to this node
    omnetpp::cModule *ptrNode = NULL;

    /** @brief Handler to the physical layer */
    MacToPhyInterface* phy;

    /** @brief Handler to the statistics module */
    VENTOS::Statistics* STAT;

    /** @brief Self message to indicate that the current channel shall be switched.*/
    omnetpp::cMessage* nextChannelSwitch = NULL;

    /** @brief Self message to wake up at next MacEvent */
    omnetpp::cMessage* nextMacEvent = NULL;

    /** @brief Last time the channel went idle */
    omnetpp::simtime_t lastIdle;
    omnetpp::simtime_t lastBusy;

    /** @brief Current state of the channel selecting operation.*/
    t_channel activeChannel;

    /** @brief access category of last sent packet */
    t_access_category lastAC;

    /** @brief Stores the frequencies in Hz that are associated to the channel numbers.*/
    std::map<int, double> frequency;

    bool useSCH;
    int mySCH;

    std::map<t_channel, EDCA*> myEDCA;

    bool idleChannel = true;

    /** @brief stats */
    long NumDroppedFrames = 0;
    long NumTooLittleTime = 0;
    long NumInternalContention = 0;
    long NumBackoff = 0;
    long SlotsBackoff = 0;
    omnetpp::simtime_t TotalBusyTime = 0;

    //tell to anybody which is interested when the channel turns busy or idle
    omnetpp::simsignal_t sigChannelBusy;
    //tell to anybody which is interested when a collision occurred
    omnetpp::simsignal_t sigCollision;

public:

    Mac1609_4() : BaseLayer(), phy(NULL) { }
    Mac1609_4(unsigned stacksize) : BaseLayer(stacksize), phy(NULL) { }
    ~Mac1609_4();

    // ######### implementations of WaveAppToMac1609_4Interface ############

    // returns the MAC address of this MAC module
    const LAddress::L2Type& getMACAddress() { return myMacAddress; }

    // return true if alternate access is enabled
    bool isChannelSwitchingActive();

    omnetpp::simtime_t getSwitchingInterval();

    bool isCurrentChannelCCH();

    void changeServiceChannel(int channelNumber);

    // #####################################################################

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

private:

    /** @brief decapsulate the network message from the MacPkt */
    virtual omnetpp::cPacket* decapsMsg(MacPkt*);

    /** @brief Encapsulate WSM into a MacPkt */
    Mac80211Pkt* encapsMsg(WaveShortMessage *wsm);

    /** @brief Set a state for the channel selecting operation.*/
    void setActiveChannel(t_channel state);

    omnetpp::simtime_t timeLeftInSlot() const;
    omnetpp::simtime_t timeLeftTillGuardOver() const;

    bool guardActive() const;

    /** @brief maps a application layer priority (up) to an EDCA access category. */
    t_access_category mapPriority(int prio);

    void channelBusy();
    void channelBusySelf(bool generateTxOp);
    void channelIdle(bool afterSwitch = false);

    void record_MAC_stat_func();
};

}

#endif /* ___MAC1609_4_H_*/

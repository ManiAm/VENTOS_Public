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
#include "PhyLayer80211p.h"
#include "WaveAppToMac1609_4Interface.h"
#include "FindModule.h"
#include "ConstsPhy.h"
#include "Mac1609_4_EDCA.h"
#include "Mac80211Pkt_m.h"
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
        /** Tells the netw layer that a packet to be sent has been dropped.*/
        PACKET_DROPPED,
        /** Stores the id on which classes extending BaseMac should
         * continue their own control kinds.*/
        LAST_BASE_MAC_CONTROL_KIND,
    };

private:

    Mac80211pToPhy11pInterface* phy11p;
    VENTOS::Statistics* STAT;
    bool record_stat;

    bool emulationActive;

    /** @brief Handler to the physical layer.*/
    MacToPhyInterface* phy;

    /**
     * @brief Length of the MacPkt header
     **/
    int headerLength;

    /**
     * @brief MAC address.
     **/
    LAddress::L2Type myMacAddr;

    /** @brief debug this core module? */
    bool coreDebug;

    /** @brief The length of the phy header (in bits).
     *
     * Since the MAC layer has to create the signal for
     * a transmission it has to know the total length of
     * the packet and therefore needs the length of the
     * phy header.
     */
    int phyHeaderLength;

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

    bool useSCH;
    int mySCH;

    std::map<t_channel, EDCA*> myEDCA;

    bool idleChannel = true;

    /** @brief stats */
    long statsReceivedPackets = 0;
    long statsReceivedBroadcasts = 0;
    long statsSentPackets = 0;
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

    Mac1609_4() : BaseLayer(), phy(NULL), myMacAddr(LAddress::L2NULL()) { }
    Mac1609_4(unsigned stacksize) : BaseLayer(stacksize), phy(NULL), myMacAddr(LAddress::L2NULL()) { }
    ~Mac1609_4() { };

    /**
     * @brief Returns the MAC address of this MAC module.
     */
    const LAddress::L2Type& getMACAddress() { return myMacAddr; }

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

    /**
     * @brief Registers this bridge's NIC with INET's InterfaceTable.
     */
    virtual void registerInterface();

    /** @brief decapsulate the network message from the MacPkt */
    virtual omnetpp::cPacket* decapsMsg(MacPkt*);

    /** @brief Encapsulate the NetwPkt into an MacPkt */
    virtual MacPkt* encapsMsg(omnetpp::cPacket*);

    /** @brief Set a state for the channel selecting operation.*/
    void setActiveChannel(t_channel state);

    omnetpp::simtime_t timeLeftInSlot() const;
    omnetpp::simtime_t timeLeftTillGuardOver() const;

    bool guardActive() const;

    Signal* createSignal(omnetpp::simtime_t start, omnetpp::simtime_t length, double power, uint64_t bitrate, double frequency);

    /** @brief maps a application layer priority (up) to an EDCA access category. */
    t_access_category mapPriority(int prio);

    void channelBusy();
    void channelBusySelf(bool generateTxOp);
    void channelIdle(bool afterSwitch = false);

    void setParametersForBitrate(uint64_t bitrate);

    omnetpp::simtime_t getFrameDuration(int payloadLengthBits, enum PHY_MCS mcs = MCS_DEFAULT) const;

    /**
     * @brief Creates a simple Signal defined over time with the
     * passed parameters.
     *
     * Convenience method to be able to create the appropriate
     * Signal for the MacToPhyControlInfo without needing to care
     * about creating Mappings.
     *
     * NOTE: The created signal's transmission-power is a rectangular function.
     * This method uses MappingUtils::addDiscontinuity to represent the discontinuities
     * at the beginning and end of this rectangular function.
     * Because of this the created mapping which represents the signal's
     * transmission-power is still zero at the exact start and end.
     * Please see the method MappingUtils::addDiscontinuity for the reason.
     */
    virtual Signal* createSimpleSignal(omnetpp::simtime_t_cref start, omnetpp::simtime_t_cref length, double power, double bitrate);

    /**
     * @brief Creates a simple Mapping with a constant curve
     * progression at the passed value.
     *
     * Used by "createSimpleSignal" to create the bitrate mapping.
     */
    Mapping* createConstantMapping(omnetpp::simtime_t_cref start, omnetpp::simtime_t_cref end, Argument::mapped_type_cref value);

    /**
     * @brief Creates a simple Mapping with a constant curve
     * progression at the passed value and discontinuities at the boundaries.
     *
     * Used by "createSimpleSignal" to create the power mapping.
     */
    Mapping* createRectangleMapping(omnetpp::simtime_t_cref start, omnetpp::simtime_t_cref end, Argument::mapped_type_cref value);

    /**
     * @brief Creates a Mapping defined over time and frequency with
     * constant power in a certain frequency band.
     */
    ConstMapping* createSingleFrequencyMapping(omnetpp::simtime_t_cref start, omnetpp::simtime_t_cref end, Argument::mapped_type_cref centerFreq, Argument::mapped_type_cref bandWith, Argument::mapped_type_cref value);

    /**
     * @brief Returns a pointer to this MACs NICs ConnectionManager module.
     * @return pointer to the connection manager module
     */
    BaseConnectionManager* getConnectionManager();

    /**
     * @brief Extracts the MAC address from the "control info" structure (object).
     *
     * Extract the destination MAC address from the "control info" which was prev. set by NetwToMacControlInfo::setControlInfo().
     *
     * @param pCtrlInfo The "control info" structure (object) prev. set by NetwToMacControlInfo::setControlInfo().
     * @return The MAC address of message receiver.
     */
    virtual const LAddress::L2Type& getUpperDestinationFromControlInfo(const cObject *const pCtrlInfo);

    /**
     * @brief Attaches a "control info" (MacToNetw) structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the destination MAC address.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg      The message where the "control info" shall be attached.
     * @param pSrcAddr  The MAC address of the message receiver.
     */
    virtual omnetpp::cObject *const setUpControlInfo(omnetpp::cMessage *const pMsg, const LAddress::L2Type& pSrcAddr);

    /**
     * @brief Attaches a "control info" (MacToPhy) structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the signal.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg      The message where the "control info" shall be attached.
     * @param pSignal   The signal which should be send.
     */
    virtual omnetpp::cObject *const setDownControlInfo(omnetpp::cMessage *const pMsg, Signal *const pSignal);

private:

    void record_MAC_stat_func();
};

}

#endif /* ___MAC1609_4_H_*/

//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
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

#ifndef PHYLAYER80211P_H_
#define PHYLAYER80211P_H_

#include "BaseWorldUtility.h"
#include "BaseConnectionManager.h"
#include "Move.h"
#include "ChannelAccess.h"
#include "ChannelInfo.h"
#include "Decider80211p.h"
#include "SNRThresholdDecider.h"
#include "Mac80211pToPhy11pInterface.h"
#include "Decider80211pToPhy80211pInterface.h"
#include "global/Statistics.h"

#ifndef DBG
#define DBG EV
#endif
//#define DBG std::cerr << "[" << simTime().raw() << "] " << getParentModule()->getFullPath() << " "

namespace Veins {

/**
 * @brief The BasePhyLayer represents the physical layer of a nic.
 *
 * The BasePhyLayer is directly connected to the mac layer via
 * OMNeT channels and is able to send messages to other physical
 * layers through sub-classing from ChannelAcces.
 *
 * The BasePhyLayer encapsulates two sub modules.
 * The AnalogueModels, which are responsible for simulating
 * the attenuation of received signals and the Decider which
 * provides the main functionality of the physical layer like
 * signal classification (noise or not noise) and demodulation
 * (calculating transmission errors).
 *
 * The BasePhyLayer itself is responsible for the OMNeT
 * depended parts of the physical layer which are the following:
 *
 * Module initialization:
 * - read ned-parameters and initialize module, Decider and
 *   AnalogueModels.
 *
 * Message handling:
 * - receive messages from mac layer and hand them to the Decider
 *   or directly send them to the channel
 * - receive AirFrames from the channel, hand them to the
 *   AnalogueModels for filtering, simulate delay and transmission
 *   duration, hand it to the Decider for evaluation and send
 *   received packets to the mac layer
 * - keep track of currently active AirFrames on the channel
 *   (see ChannelInfo)
 *
 * The actual evaluation of incoming signals is done by the
 * Decider.
 *
 * base class ChannelAccess:
 * - provides access to the channel via the ConnectionManager
 *
 * base class DeciderToPhyInterface:
 * - interface for the Decider
 *
 * base class MacToPhyInterface:
 * - interface for the Mac
 *
 */

class PhyLayer80211p : public ChannelAccess, public DeciderToPhyInterface, public MacToPhyInterface, public Mac80211pToPhy11pInterface, public Decider80211pToPhy80211pInterface
{
private:

    VENTOS::Statistics* STAT;
    /** @brief Pointer to the World Utility, to obtain some global information*/
    BaseWorldUtility* world;

    long NumSentFrames = 0;
    long NumReceivedFrames = 0;
    long NumLostFrames_BiteError = 0;  // A frame was not received due to bit-errors
    long NumLostFrames_Collision = 0;  // A frame was not received due to collision
    long NumLostFrames_TXRX = 0;       // A frame was not received because we were sending while receiving

    bool record_stat;
    bool record_frameTxRx;

    std::string myId;

    int protocolId;

    enum ProtocolIds {
        GENERIC = 0,
        IEEE_80211 = 12123
    };

    bool emulationActive;

    /** @brief Defines the strength of the thermal noise.*/
    ConstantSimpleConstMapping* thermalNoise;

    /** @brief The sensitivity describes the minimum strength a signal must have to be received.*/
    double sensitivity;

    /** @brief Stores if tracking of statistics (esp. cOutvectors) is enabled.*/
    bool recordStats;

    /**
     * @brief Channel info keeps track of received AirFrames and provides information about
     * currently active AirFrames at the channel.
     */
    ChannelInfo channelInfo;

    /** @brief The state machine storing the current radio state (TX, RX, SLEEP).*/
    Radio* radio;

    /** @brief Pointer to the decider module. */
    Decider* decider;

    /** @brief Used to store the AnalogueModels to be used as filters.*/
    typedef std::vector<AnalogueModel*> AnalogueModelList;

    /** @brief List of the analogue models to use.*/
    AnalogueModelList analogueModels;

    /**
     * @brief Used at initialisation to pass the parameters
     * to the AnalogueModel and Decider
     */
    typedef std::map<std::string, omnetpp::cMsgPar> ParameterMap;

    /** @brief The id of the in-data gate from the Mac layer */
    int upperLayerIn;
    /** @brief The id of the out-data gate to the Mac layer */
    int upperLayerOut;
    /** @brief The id of the out-control gate to the Mac layer */
    int upperControlOut;
    /** @brief The id of the in-control gate from the Mac layer */
    int upperControlIn;

    /**
     * @brief Self message scheduled to the point in time when the
     * switching process of the radio is over.
     */
    omnetpp::cMessage* radioSwitchingOverTimer;

    /**
     * @brief Self message scheduled to the point in time when the
     * transmission of an AirFrame is over.
     */
    omnetpp::cMessage* txOverTimer;

    /** @brief The states of the receiving process for AirFrames.*/
    enum AirFrameStates {
        /** @brief Start of actual receiving process of the AirFrame. */
        START_RECEIVE = 1,
        /** @brief AirFrame is being received. */
        RECEIVING,
        /** @brief Receiving process over */
        END_RECEIVE
    };

    /** @brief Stores the length of the phy header in bits. */
    int headerLength;

    /** @brief CCA threshold. See Decider80211p for details */
    double ccaThreshold;

    /** @brief enable/disable detection of packet collisions */
    bool collectCollisionStatistics;

    /** @brief allows/disallows interruption of current reception for txing
     *
     * See detailed description in Decider80211p
     */
    bool allowTxDuringRx;

private:

    void record_PHY_stat_func();
    void record_frameTxRx_stat_func(VENTOS::PhyToMacReport* msg, std::string report);

public:

    PhyLayer80211p();

    /**
     * Free the pointer to the decider and the AnalogueModels and the Radio.
     */
    virtual ~PhyLayer80211p();

    /**
     * @brief OMNeT++ initialization function.
     *
     * Read simple parameters.
     * Read and parse xml file for decider and analogue models
     * configuration.
     */
    void initialize(int stage);

    /** @brief Only calls the deciders finish method.*/
    virtual void finish();

    /**
     * @brief OMNeT++ handle message function.
     *
     * Classify and forward message to subroutines.
     * - AirFrames from channel
     * - self scheduled AirFrames
     * - MacPackets from MAC layer
     * - ControllMesasges from MAC layer
     * - self messages like TX_OVER and RADIO_SWITCHED
     */
    virtual void handleMessage(omnetpp::cMessage* msg);

    /**
     * @brief Set the carrier sense threshold
     * @param ccaThreshold_dBm the cca threshold in dBm
     */
    void setCCAThreshold(double ccaThreshold_dBm);

    /**
     * @brief Return the cca threshold in dBm
     */
    double getCCAThreshold();

    /**
     * @brief Called by the Decider to send a control message to the MACLayer
     *
     * This function can be used to answer a ChannelSenseRequest to the MACLayer
     *
     */
    void sendControlMsgToMac(VENTOS::PhyToMacReport* msg);

    /**
     * @brief Called to send an AirFrame with DeciderResult to the MACLayer
     *
     * When a packet is completely received and not noise, the Decider
     * call this function to send the packet together with
     * the corresponding DeciderResult up to MACLayer
     *
     */
    void sendUp(AirFrame* packet, DeciderResult* result);

    /**
     * @brief Tells the PhyLayer to cancel a scheduled message (AirFrame or
     * ControlMessage).
     *
     * Used by the Decider if it doesn't need to handle an AirFrame or
     * ControlMessage again anymore.
     */
    virtual void cancelScheduledMessage(omnetpp::cMessage* msg);

    /**
     * @brief Tells the PhyLayer to reschedule a message (AirFrame or
     * ControlMessage).
     *
     * Used by the Decider if it has to handle an AirFrame or an control message
     * earlier than it has returned to the PhyLayer the last time the Decider
     * handled that message.
     */
    virtual void rescheduleMessage(omnetpp::cMessage* msg, omnetpp::simtime_t_cref t);

    /**
     * @brief Returns a pointer to the simulations world-utility-module.
     */
    virtual BaseWorldUtility* getWorldUtility();

    /**
     * @brief Records a double into the scalar result file.
     *
     * Implements the method from DeciderToPhyInterface, method-calls are forwarded
     * to OMNeT-method 'recordScalar'.
     */
    void recordScalar(const char *name, double value, const char *unit=NULL);

    /*@}*/

    /**
     * @brief Attaches a "control info" (PhyToMac) structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the decider result.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg      The message where the "control info" shall be attached.
     * @param pSrcAddr  The MAC address of the message receiver.
     */
    virtual omnetpp::cObject *const setUpControlInfo(omnetpp::cMessage *const pMsg, DeciderResult *const pDeciderResult);

private:

    /** @brief Defines the scheduling priority of AirFrames.
     *
     * AirFrames use a slightly higher priority than normal to ensure
     * channel consistency. This means that before anything else happens
     * at a time point t every AirFrame which ended at t has been removed and
     * every AirFrame started at t has been added to the channel.
     *
     * An example where this matters is a ChannelSenseRequest which ends at
     * the same time as an AirFrame starts (or ends). Depending on which message
     * is handled first the result of ChannelSenseRequest would differ.
     */
    static short airFramePriority() {
        return 10;
    }

    /**
     * @brief Utility function. Reads the parameters of a XML element
     * and stores them in the passed ParameterMap reference.
     */
    void getParametersFromXML(omnetpp::cXMLElement* xmlData, ParameterMap& outputMap);

    /**
     * @brief Creates and returns an instance of the AnalogueModel with the
     * specified name.
     *
     * Is able to initialize the following AnalogueModels:
     */
    virtual AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);

    /**
     * @brief Initializes and returns the radio class to use.
     *
     * Can be overridden by sub-classing phy layers to use their
     * own Radio implementations.
     */
    virtual Radio* initializeRadio();

    /**
     * @brief Initializes the AnalogueModels with the data from the
     * passed XML-config data.
     */
    void initializeAnalogueModels(omnetpp::cXMLElement* xmlConfig);

    /**
     * @brief Initializes the Decider with the data from the
     * passed XML-config data.
     */
    void initializeDecider(omnetpp::cXMLElement* xmlConfig);

    /**
     * @brief Creates and initializes a SimplePathlossModel with the
     * passed parameter values.
     */
    AnalogueModel* initializeSimplePathlossModel(ParameterMap& params);

    /**
     * @brief Creates and initializes a LogNormalShadowing with the
     * passed parameter values.
     */
    AnalogueModel* initializeLogNormalShadowing(ParameterMap& params);

    /**
     * @brief Creates and initializes a JakesFading with the
     * passed parameter values.
     */
    AnalogueModel* initializeJakesFading(ParameterMap& params);

    /**
     * @brief Creates and initializes a BreakpointPathlossModel with the
     * passed parameter values.
     */
    virtual AnalogueModel* initializeBreakpointPathlossModel(ParameterMap& params);

    /**
     * @brief Creates and initializes a SimpleObstacleShadowing with the
     * passed parameter values.
     */
    AnalogueModel* initializeSimpleObstacleShadowing(ParameterMap& params);

    /**
     * @brief Creates a simple Packet Error Rate model that attenuates a percentage
     * of the packets to zero, and does not attenuate the other packets.
     *
     */
    virtual AnalogueModel* initializePERModel(ParameterMap& params);

    /**
     * @brief Creates and initializes a TwoRayInterferenceModel with the
     * passed parameter values.
     */
    AnalogueModel* initializeTwoRayInterferenceModel(ParameterMap& params);

    /**
     * @brief Creates and initializes a NakagamiFading with the
     * passed parameter values.
     */
    AnalogueModel* initializeNakagamiFading(ParameterMap& params);

    /**
     * @brief Creates and returns an instance of the Decider with the specified name.
     *
     * Is able to initialize the following Deciders:
     *
     * - Decider80211p
     * - SNRThresholdDecider
     */
    virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);

    /**
     * @brief Initializes a new Decider80211 from the passed parameter map.
     */
    virtual Decider* initializeDecider80211p(ParameterMap& params);

    /**
     * @brief This function encapsulates messages from the upper layer into an
     * AirFrame and sets all necessary attributes.
     */
    virtual AirFrame *encapsMsg(omnetpp::cPacket *msg);

    /**
     * @brief Filters the passed AirFrame's Signal by every registered AnalogueModel.
     */
    virtual void filterSignal(AirFrame *frame);

    /**
     * @brief Called the moment the simulated switching process of the Radio is finished.
     *
     * The Radio is set the new RadioState and the MAC Layer is sent
     * a confirmation message.
     */
    virtual void finishRadioSwitching();

    /**
     * @name Handle Messages
     **/
    /*@{ */
    /**
     * @brief Handles messages received from the channel (probably AirFrames).
     */
    virtual void handleAirFrame(AirFrame* frame);

    virtual void handleUpperMessage(omnetpp::cMessage* msg);

    /**
     * @brief Handles reception of a ChannelSenseRequest by forwarding it
     * to the decider and scheduling it to the point in time
     * returned by the decider.
     */
    virtual void handleChannelSenseRequest(omnetpp::cMessage* msg);

    /**
     * @brief Handles messages received from the upper layer through the
     * control gate.
     */
    virtual void handleUpperControlMessage(omnetpp::cMessage* msg);

    /**
     * @brief Handles incoming AirFrames with the state FIRST_RECEIVE.
     */
    void handleAirFrameFirstReceive(AirFrame* msg);

    /**
     * @brief Handles incoming AirFrames with the state START_RECEIVE.
     */
    virtual void handleAirFrameStartReceive(AirFrame* msg);

    /**
     * @brief Handles incoming AirFrames with the state RECEIVING.
     */
    virtual void handleAirFrameReceiving(AirFrame* msg);

    /**
     * @brief Handles incoming AirFrames with the state END_RECEIVE.
     */
    virtual void handleAirFrameEndReceive(AirFrame* msg);

    virtual void changeListeningFrequency(double freq);

    /**
     * @brief Handles self scheduled messages.
     */
    virtual void handleSelfMessage(omnetpp::cMessage* msg);

    /**
     * @brief Returns the identifier of the protocol this phy uses to send
     * messages.
     *
     * @return An integer representing the identifier of the used protocol.
     */
    virtual int myProtocolId() { return protocolId; }

    /**
     * @brief Returns true if the protocol with the passed identifier is
     * decodeable by the decider.
     *
     * If the protocol with the passed id is not understood by this phy layers
     * decider the according AirFrame is not passed to the it but only is added
     * to channel info to be available as interference to the decider.
     *
     * Default implementation checks only if the passed id is the same as the
     * one returned by "myProtocolId()".
     *
     * @param id The identifier of the protocol of an AirFrame.
     * @return Returns true if the passed protocol id is supported by this phy-
     * layer.
     */
    virtual bool isKnownProtocolId(int id) { return id == myProtocolId(); }

    //---------MacToPhyInterface implementation-----------
    /**
     * @name MacToPhyInterface implementation
     * @brief These methods implement the MacToPhyInterface.
     **/
    /*@{ */

    /**
     * @brief Returns the current state the radio is in.
     *
     * See RadioState for possible values.
     *
     * This method is mainly used by the mac layer.
     */
    virtual int getRadioState();

    /**
     * @brief Tells the BasePhyLayer to switch to the specified
     * radio state.
     *
     * The switching process can take some time depending on the
     * specified switching times in the ned file.
     *
     * @return  -1: Error code if the Radio is currently switching
     *          else: switching time from the current RadioState to the new RadioState
     */
    virtual omnetpp::simtime_t setRadioState(int rs);

    /** @brief Sets the channel currently used by the radio. */
    virtual void setCurrentRadioChannel(int newRadioChannel);

    /** @brief Returns the channel currently used by the radio. */
    virtual int getCurrentRadioChannel();

    /** @brief Returns the number of channels available on this radio. */
    virtual int getNbRadioChannels();

    /**
     * @brief Fills the passed AirFrameVector with all AirFrames that intersect
     * with the time interval [from, to]
     */
    virtual void getChannelInfo(omnetpp::simtime_t_cref from, omnetpp::simtime_t_cref to, AirFrameVector& out);

    /**
     * @brief Returns a Mapping which defines the thermal noise in
     * the passed time frame (in mW).
     *
     * The implementing class of this method keeps ownership of the
     * Mapping.
     *
     * This implementation returns a constant mapping with the value
     * of the "thermalNoise" module parameter
     *
     * Override this method if you want to define a more complex
     * thermal noise.
     */
    virtual ConstMapping* getThermalNoise(omnetpp::simtime_t_cref from, omnetpp::simtime_t_cref to);

    /**
     * @brief Returns the current state of the channel.
     *
     * See ChannelState for details.
     */
    virtual ChannelState getChannelState();

    /**
     * @brief Returns the length of the phy header in bits.
     *
     * Since the MAC layer has to create the signal for
     * a transmission it has to know the total length of
     * the packet and therefore needs the length of the
     * phy header.
     */
    virtual int getPhyHeaderLength();
};

}

#endif /* PHYLAYER80211P_H_ */

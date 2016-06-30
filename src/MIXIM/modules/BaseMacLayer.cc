/***************************************************************************
 * file:        BaseMacLayer.cc
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: basic MAC layer class
 *              subclass to create your own MAC layer
 **************************************************************************/

#include <cassert>
#include <sstream>

#include "BaseMacLayer.h"
#include "Mapping.h"
#include "Signal_.h"
#include "MacToPhyInterface.h"
#include "MacToNetwControlInfo.h"
#include "NetwToMacControlInfo.h"
#include "MacToPhyControlInfo.h"
#include "AddressingInterface.h"
#include "ChannelAccess.h"
#include "FindModule.h"
#include "MacPkt_m.h"

Define_Module(BaseMacLayer);

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BaseLayer.
 *
 **/
void BaseMacLayer::initialize(int stage)
{
    BaseLayer::initialize(stage);

    if(stage==0)
    {
        // get handle to phy layer
        if ((phy = FindModule<MacToPhyInterface*>::findSubModule(getParentModule())) == NULL) {
            throw omnetpp::cRuntimeError("Could not find a PHY module.");
        }
        headerLength    = par("headerLength");
        phyHeaderLength = phy->getPhyHeaderLength();

        hasPar("coreDebug") ? coreDebug = par("coreDebug").boolValue() : coreDebug = false;
    }
    if (myMacAddr == LAddress::L2NULL()) {
        // see if there is an addressing module available
        // otherwise use NIC modules id as MAC address
        AddressingInterface* addrScheme = FindModule<AddressingInterface*>::findSubModule(findHost());
        if(addrScheme) {
            myMacAddr = addrScheme->myMacAddr(this);
        } else {
            const std::string addressString = par("address").stringValue();
            if (addressString.empty() || addressString == "auto")
                myMacAddr = LAddress::L2Type(getParentModule()->getId());
            else
                myMacAddr = LAddress::L2Type(addressString.c_str());
            // use streaming operator for string conversion, this makes it more
            // independent from the myMacAddr type
            std::ostringstream oSS; oSS << myMacAddr;
            par("address").setStringValue(oSS.str());
        }
        registerInterface();
    }
}

void BaseMacLayer::registerInterface()
{
}

/**
 * Decapsulates the network packet from the received MacPkt
 **/
omnetpp::cPacket* BaseMacLayer::decapsMsg(MacPkt* msg)
{
    omnetpp::cPacket *m = msg->decapsulate();
    setUpControlInfo(m, msg->getSrcAddr());
    // delete the macPkt
    delete msg;
    coreEV << " message decapsulated " << std::endl;
    return m;
}

/**
 * Encapsulates the received NetwPkt into a MacPkt and set all needed
 * header fields.
 **/
MacPkt* BaseMacLayer::encapsMsg(omnetpp::cPacket *netwPkt)
{
    MacPkt *pkt = new MacPkt(netwPkt->getName(), netwPkt->getKind());
    pkt->setBitLength(headerLength);

    // copy dest address from the Control Info attached to the network
    // message by the network layer
    cObject* cInfo = netwPkt->removeControlInfo();

    coreEV <<"CInfo removed, mac addr="<< getUpperDestinationFromControlInfo(cInfo) << std::endl;
    pkt->setDestAddr(getUpperDestinationFromControlInfo(cInfo));

    //delete the control info
    delete cInfo;

    //set the src address to own mac address (nic module getId())
    pkt->setSrcAddr(myMacAddr);

    //encapsulate the network packet
    pkt->encapsulate(netwPkt);
    coreEV <<"pkt encapsulated\n";

    return pkt;
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to lower layers.
 *
 * To forward the message to lower layers after processing it please
 * use @ref sendDown. It will take care of anything needed
 **/
void BaseMacLayer::handleUpperMsg(omnetpp::cMessage *mac)
{
    assert(dynamic_cast<omnetpp::cPacket*>(mac));
    sendDown(encapsMsg(static_cast<omnetpp::cPacket*>(mac)));
}

/**
 * This basic implementation just forwards all message that are
 * broadcast (destAddr = L2BROADCAST) or destined for this node
 * (destAddr = nic module getId()) to the network layer
 *
 * @sa sendUp
 **/

void BaseMacLayer::handleLowerMsg(omnetpp::cMessage *msg)
{
    MacPkt*          mac  = static_cast<MacPkt *>(msg);
    LAddress::L2Type dest = mac->getDestAddr();
    LAddress::L2Type src  = mac->getSrcAddr();

    //only foward to upper layer if message is for me or broadcast
    if((dest == myMacAddr) || LAddress::isL2Broadcast(dest)) {
        coreEV << "message with mac addr " << src
                << " for me (dest=" << dest
                << ") -> forward packet to upper layer\n";
        sendUp(decapsMsg(mac));
    }
    else{
        coreEV << "message with mac addr " << src
                << " not for me (dest=" << dest
                << ") -> delete (my MAC="<<myMacAddr<<")\n";
        delete mac;
    }
}

void BaseMacLayer::handleLowerControl(omnetpp::cMessage* msg)
{
    switch (msg->getKind())
    {
    case MacToPhyInterface::TX_OVER:
        msg->setKind(TX_OVER);
        sendControlUp(msg);
        break;
    default:
        EV << "BaseMacLayer does not handle control messages of this type (name was "<<msg->getName()<<")\n";
        delete msg;
        break;
    }
}

Signal* BaseMacLayer::createSimpleSignal(omnetpp::simtime_t_cref start, omnetpp::simtime_t_cref length, double power, double bitrate)
{
    omnetpp::simtime_t end = start + length;
    //create signal with start at current simtime and passed length
    Signal* s = new Signal(start, length);

    //create and set tx power mapping
    Mapping* txPowerMapping = createRectangleMapping(start, end, power);
    s->setTransmissionPower(txPowerMapping);

    //create and set bitrate mapping
    Mapping* bitrateMapping = createConstantMapping(start, end, bitrate);
    s->setBitrate(bitrateMapping);

    return s;
}

Mapping* BaseMacLayer::createConstantMapping(omnetpp::simtime_t_cref start, omnetpp::simtime_t_cref end, Argument::mapped_type_cref value)
{
    //create mapping over time
    Mapping* m = MappingUtils::createMapping(Argument::MappedZero(), DimensionSet::timeDomain(), Mapping::LINEAR);

    //set position Argument
    Argument startPos(start);

    //set mapping at position
    m->setValue(startPos, value);

    //set position Argument
    Argument endPos(end);

    //set mapping at position
    m->setValue(endPos, value);

    return m;
}

Mapping* BaseMacLayer::createRectangleMapping(omnetpp::simtime_t_cref start, omnetpp::simtime_t_cref end, Argument::mapped_type_cref value)
{
    //create mapping over time
    Mapping* m = MappingUtils::createMapping(DimensionSet::timeDomain(), Mapping::LINEAR);

    //set position Argument
    Argument startPos(start);
    //set discontinuity at position
    MappingUtils::addDiscontinuity(m, startPos, Argument::MappedZero(), MappingUtils::post(start), value);

    //set position Argument
    Argument endPos(end);
    //set discontinuity at position
    MappingUtils::addDiscontinuity(m, endPos, Argument::MappedZero(), MappingUtils::pre(end), value);

    return m;
}

ConstMapping* BaseMacLayer::createSingleFrequencyMapping(omnetpp::simtime_t_cref             start,
        omnetpp::simtime_t_cref             end,
        Argument::mapped_type_cref centerFreq,
        Argument::mapped_type_cref halfBandwidth,
        Argument::mapped_type_cref value)
{
    Mapping* res = MappingUtils::createMapping(Argument::MappedZero(), DimensionSet::timeFreqDomain(), Mapping::LINEAR);

    Argument pos(DimensionSet::timeFreqDomain());

    pos.setArgValue(Dimension::frequency(), centerFreq - halfBandwidth);
    pos.setTime(start);
    res->setValue(pos, value);

    pos.setTime(end);
    res->setValue(pos, value);

    pos.setArgValue(Dimension::frequency(), centerFreq + halfBandwidth);
    res->setValue(pos, value);

    pos.setTime(start);
    res->setValue(pos, value);

    return res;
}

BaseConnectionManager* BaseMacLayer::getConnectionManager() {
    cModule* nic = getParentModule();
    return ChannelAccess::getConnectionManager(nic);
}

const LAddress::L2Type& BaseMacLayer::getUpperDestinationFromControlInfo(const cObject *const pCtrlInfo) {
    return NetwToMacControlInfo::getDestFromControlInfo(pCtrlInfo);
}

/**
 * Attaches a "control info" (MacToNetw) structure (object) to the message pMsg.
 */
omnetpp::cObject *const BaseMacLayer::setUpControlInfo(omnetpp::cMessage *const pMsg, const LAddress::L2Type& pSrcAddr)
{
    return MacToNetwControlInfo::setControlInfo(pMsg, pSrcAddr);
}

/**
 * Attaches a "control info" (MacToPhy) structure (object) to the message pMsg.
 */
omnetpp::cObject *const BaseMacLayer::setDownControlInfo(omnetpp::cMessage *const pMsg, Signal *const pSignal)
{
    return MacToPhyControlInfo::setControlInfo(pMsg, pSignal);
}

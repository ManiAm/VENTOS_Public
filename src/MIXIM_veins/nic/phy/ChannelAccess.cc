/***************************************************************************
 * file:        ChannelAccess.cc
 *
 * author:      Marc Loebbers
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
 * description: - Base class for physical layers
 *              - if you create your own physical layer, please subclass
 *                from this class and use the sendToChannel() function!!
 ***************************************************************************
 * changelog:   $Revision: 284 $
 *              last modified:   $Date: 2006-06-07 16:55:24 +0200 (Mi, 07 Jun 2006) $
 *              by:              $Author: willkomm $
 **************************************************************************/

#include <cassert>

#include "ChannelAccess.h"
#include "global/FindModule.h"
#include "global/BaseWorldUtility.h"
#include "MIXIM_veins/connectionManager/BaseConnectionManager.h"
#include "msg/AirFrame11p_serial.h"
#include "msg/WaveShortMessage_m.h"
#include "MIXIM_veins/nic/mac/MacToPhyControlInfo.h"

const simsignalwrap_t ChannelAccess::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);


void ChannelAccess::initialize(int stage)
{
    BaseModule::initialize(stage);

    if(stage == 0)
    {
        coreDebug = hasPar("coreDebug") ? par("coreDebug").boolValue() : false;

        if(this->getParentModule()->getParentModule()->par("DSRCenabled"))
            findHost()->subscribe(mobilityStateChangedSignal, this);

        // get a pointer to the connection manager module
        omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("connMan");
        cc = static_cast<BaseConnectionManager*>(module);
        ASSERT(cc);

        // get a pointer to the Statistics module
        module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("statistics");
        STAT = static_cast<VENTOS::Statistics*>(module);
        ASSERT(STAT);
    }
    else if (stage == 1)
    {
        record_frameTxRx = this->par("record_frameTxRx").boolValue();

        usePropagationDelay = par("usePropagationDelay");
    }
}


void ChannelAccess::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, omnetpp::cObject *obj, cObject* details)
{
    if(signalID == mobilityStateChangedSignal)
    {
        ChannelMobilityPtrType const mobility = omnetpp::check_and_cast<ChannelMobilityPtrType>(obj);
        Coord pos = mobility->getCurrentPosition();

        if(isRegistered)
            // getParentModule points to the 'nic' module
            cc->updateNicPos(getParentModule()->getId(), &pos);
        else
        {
            // register the nic with ConnectionManager
            // returns true, if sendDirect is used
            useSendDirect = cc->registerNic(getParentModule(), this, &pos);
            isRegistered  = true;
        }
    }
}


void ChannelAccess::sendToChannel(omnetpp::cPacket *msg)
{
    const NicEntry::GateList& gateList = cc->getGateList(getParentModule()->getId());
    NicEntry::GateList::const_iterator i = gateList.begin();

    if(useSendDirect)
    {
        // use Andras stuff
        if( i != gateList.end() )
        {
            omnetpp::simtime_t_cref frameDuration = (dynamic_cast<AirFrame*> (msg))->getDuration();

            for(; i != --gateList.end(); ++i)
            {
                // calculate propagation delay to this receiving nic
                auto prop = calculatePropagationDelay(i->first);

                int radioStart = i->second->getId();
                int radioEnd = radioStart + i->second->size();
                for (int g = radioStart; g != radioEnd; ++g)
                {
                    // in each iteration, we need to duplicate the msg before sending
                    omnetpp::cPacket *msg_dup = msg->dup();

                    if(record_frameTxRx)
                        recordFrameTx(msg_dup, i->second, prop);

                    sendDirect(static_cast<omnetpp::cPacket*>(msg_dup), prop.propagationDelay, frameDuration, i->second->getOwnerModule(), g);
                }
            }

            // calculate Propagation delay to this receiving nic
            auto prop = calculatePropagationDelay(i->first);

            int radioStart = i->second->getId();
            int radioEnd = radioStart + i->second->size();
            for (int g = radioStart; g != --radioEnd; ++g)
            {
                // in each iteration, we need to duplicate the msg before sending
                omnetpp::cPacket *msg_dup = msg->dup();

                if(record_frameTxRx)
                    recordFrameTx(msg_dup, i->second, prop);

                sendDirect(static_cast<omnetpp::cPacket*>(msg_dup), prop.propagationDelay, frameDuration, i->second->getOwnerModule(), g);
            }

            if(record_frameTxRx)
                recordFrameTx(msg, i->second, prop);

            sendDirect(msg, prop.propagationDelay, frameDuration, i->second->getOwnerModule(), radioEnd);
        }
        else
        {
            coreEV << "Nic is not connected to any gates!" << std::endl;

            if(record_frameTxRx)
            {
                prop_t entry = {-1, -1};
                recordFrameTx(msg, NULL, entry);
            }

            delete msg;
        }
    }
    else
    {
        // use our stuff
        coreEV << "sendToChannel: sending to gates \n";
        if( i != gateList.end() )
        {
            for(; i != --gateList.end(); ++i)
            {
                // calculate Propagation delay to this receiving nic
                auto prop = calculatePropagationDelay(i->first);

                sendDelayed( static_cast<omnetpp::cPacket*>(msg->dup()), prop.propagationDelay, i->second );
            }

            // calculate Propagation delay to this receiving nic
            auto prop = calculatePropagationDelay(i->first);

            sendDelayed( msg, prop.propagationDelay, i->second );
        }
        else
        {
            coreEV << "Nic is not connected to any gates!" << std::endl;
            delete msg;
        }
    }
}


ChannelAccess::prop_t ChannelAccess::calculatePropagationDelay(const NicEntry* nic)
{
    ChannelAccess *const senderModule   = this;
    ChannelAccess *const receiverModule = nic->chAccess;

    assert(senderModule);
    assert(receiverModule);

    // claim the Move pattern of the sender from the Signal
    Coord sendersPos  = senderModule->getMobilityModule()->getCurrentPosition();
    Coord receiverPos = receiverModule->getMobilityModule()->getCurrentPosition();

    // this time-point is used to calculate the distance between sending and receiving host
    double dist = receiverPos.distance(sendersPos);

    omnetpp::simtime_t propagationDelay = 0;
    if(usePropagationDelay)
        propagationDelay = dist / BaseWorldUtility::speedOfLight();

    prop_t entry = {propagationDelay, dist};

    return entry;
}


void ChannelAccess::recordFrameTx(omnetpp::cPacket *msg /*AirFrame11p*/, omnetpp::cGate *gate, prop_t propDelay)
{
    // only SendDirect is supported (for now!)
    ASSERT(useSendDirect);

    uint32_t frameId = msg->getId();  // unique message id assigned by OMNET++
    int32_t nicId = (gate != NULL) ? gate->getOwnerModule()->getSubmodule("nic")->getId() : -1;  // receiver nic

    auto it = STAT->global_frameTxRx_stat.find(std::make_pair(frameId, nicId));
    if(it != STAT->global_frameTxRx_stat.end())
        throw omnetpp::cRuntimeError("frame/nic '(%d,%d)' is not unique", frameId, nicId);
    else
    {
        Veins::AirFrame11p *frame = dynamic_cast<Veins::AirFrame11p *>(msg);
        ASSERT(frame);

        ConstMappingIterator* bitrateIt = frame->getSignal().getBitrate()->createConstIterator();
        bitrateIt->next(); // iterate to payload bitrate indicator

        ConstMappingIterator* txIt = frame->getSignal().getTransmissionPower()->createConstIterator();
        txIt->next(); // iterate to payload TxPower indicator

        int chNum = -1;

        omnetpp::cPacket *mac = frame->getEncapsulatedPacket();
        if(mac)
        {
            omnetpp::cPacket *appl = mac->getEncapsulatedPacket();
            if(appl)
            {
                Veins::WaveShortMessage *appl_class = dynamic_cast<Veins::WaveShortMessage *>(appl);
                ASSERT(appl_class);

                chNum = appl_class->getChannelNumber();
            }
        }

        VENTOS::msgTxRxStat_t entry = {};

        entry.MsgName = msg->getFullName();
        entry.SenderNode = this->getParentModule()->getParentModule()->getFullName();
        entry.ReceiverNode = (gate != NULL) ? gate->getOwnerModule()->getFullName() : "-";
        entry.SentAt = omnetpp::simTime().dbl();
        entry.FrameSize = msg->getBitLength();
        entry.channelNum = chNum;
        entry.TransmissionPower = txIt->getValue();
        entry.TransmissionSpeed = bitrateIt->getValue();
        entry.TransmissionTime = frame->getDuration().dbl();
        entry.DistanceToReceiver = propDelay.distance;
        entry.PropagationDelay = propDelay.propagationDelay.dbl();
        // these two fields are set in the receiver
        entry.ReceivedAt = -1;
        entry.FrameRxStatus = (gate != NULL) ? "" : "-";

        STAT->global_frameTxRx_stat.insert(std::make_pair(std::make_pair(frameId, nicId), entry));
    }
}

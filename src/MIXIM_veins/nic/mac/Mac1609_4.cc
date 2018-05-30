//
// Copyright (C) 2012 David Eckhoff <eckhoff@cs.fau.de>
// Second Author: Mani Amoozadeh <maniam@ucdavis.edu>
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

#include "Mac1609_4.h"
#include "MIXIM_veins/nic/phy/decider/DeciderResult80211.h"
#include "MIXIM_veins/nic/phy/PhyToMacControlInfo.h"
#include "AddressingInterface.h"
#include "MacToNetwControlInfo.h"
#include "baseAppl/ApplToPhyControlInfo.h"

namespace Veins {

Define_Module(Veins::Mac1609_4);

Mac1609_4::~Mac1609_4()
{
    cancelAndDelete(nextMacEvent);
    cancelAndDelete(nextChannelSwitch);

    // clean up queues
    for (auto &iter : myEDCA)
    {
        iter.second->cleanUp();
        delete iter.second;
    }

    myEDCA.clear();
}


void Mac1609_4::initialize(int stage)
{
    BaseLayer::initialize(stage);

    if(stage == 0)
    {
        headerLength = par("headerLength").intValue();
        hasPar("coreDebug") ? coreDebug = par("coreDebug").boolValue() : coreDebug = false;
        myMacAddress = intuniform(0,0xFFFFFFFE);
        myId = getParentModule()->getParentModule()->getFullName();
        txPower = par("txPower").doubleValue();
        bitrate = par("bitrate").intValue();
        record_stat = par("record_stat").boolValue();

        // get handle to phy layer
        phy = FindModule<MacToPhyInterface*>::findSubModule(getParentModule());
        if (phy == NULL)
            throw omnetpp::cRuntimeError("Could not find a PHY module.");

        // get a pointer to the Statistics module
        omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("statistics");
        STAT = static_cast<VENTOS::Statistics*>(module);
        ASSERT(STAT);

        // get a reference to this node
        ptrNode = this->getParentModule()->getParentModule();

        // this is required to circumvent double precision issues with constants from CONST80211p.h
        assert(omnetpp::simTime().getScaleExp() == -12);

        nextMacEvent = new omnetpp::cMessage("next Mac Event");

        sigChannelBusy = registerSignal("sigChannelBusy");
        sigCollision = registerSignal("sigCollision");

        // create frequency mappings
        frequency[Channels::CRIT_SOL] = 5.86e9;
        frequency[Channels::SCH1] = 5.87e9;
        frequency[Channels::SCH2] = 5.88e9;
        frequency[Channels::CCH] = 5.89e9;
        frequency[Channels::SCH3] = 5.90e9;
        frequency[Channels::SCH4] = 5.91e9;
        frequency[Channels::HPPS] = 5.92e9;

        // EDCA for type_CCH
        myEDCA[type_CCH] = new EDCA(this, type_CCH, par("queueSize").intValue());
        myEDCA[type_CCH]->myId = myId;
        myEDCA[type_CCH]->myId.append(" CCH");
        // creating four queues
        myEDCA[type_CCH]->createQueue(2 /*AIFS*/, (((CWMIN_11P+1)/4)-1) /*CWmin*/, (((CWMIN_11P +1)/2)-1) /*CWmax*/, AC_VO);
        myEDCA[type_CCH]->createQueue(3 /*AIFS*/, (((CWMIN_11P+1)/2)-1) /*CWmin*/, CWMIN_11P /*CWmax*/, AC_VI);
        myEDCA[type_CCH]->createQueue(6 /*AIFS*/, CWMIN_11P /*CWmin*/, CWMAX_11P /*CWmax*/, AC_BE);
        myEDCA[type_CCH]->createQueue(9 /*AIFS*/, CWMIN_11P /*CWmin*/, CWMAX_11P /*CWmax*/, AC_BK);

        // EDCA for type_SCH
        myEDCA[type_SCH] = new EDCA(this, type_SCH, par("queueSize").intValue());
        myEDCA[type_SCH]->myId = myId;
        myEDCA[type_SCH]->myId.append(" SCH");
        // creating four queues
        myEDCA[type_SCH]->createQueue(2 /*AIFS*/, (((CWMIN_11P+1)/4)-1) /*CWmin*/,(((CWMIN_11P +1)/2)-1) /*CWmax*/, AC_VO);
        myEDCA[type_SCH]->createQueue(3 /*AIFS*/, (((CWMIN_11P+1)/2)-1) /*CWmin*/, CWMIN_11P /*CWmax*/, AC_VI);
        myEDCA[type_SCH]->createQueue(6 /*AIFS*/, CWMIN_11P /*CWmin*/, CWMAX_11P /*CWmax*/, AC_BE);
        myEDCA[type_SCH]->createQueue(9 /*AIFS*/, CWMIN_11P /*CWmin*/,CWMAX_11P /*CWmax*/, AC_BK);

        useSCH = par("useServiceChannel").boolValue();

        if (useSCH)
        {
            // set the initial service channel
            switch (par("serviceChannel").intValue())
            {
            case 1: mySCH = Channels::SCH1; break;
            case 2: mySCH = Channels::SCH2; break;
            case 3: mySCH = Channels::SCH3; break;
            case 4: mySCH = Channels::SCH4; break;
            default: throw omnetpp::cRuntimeError("Service Channel must be between 1 and 4"); break;
            }

            uint64_t currenTime = omnetpp::simTime().raw();
            uint64_t switchingTime = SWITCHING_INTERVAL_11P.raw();
            double timeToNextSwitch = (double)(switchingTime - (currenTime % switchingTime)) / omnetpp::simTime().getScale();

            if ( (currenTime / switchingTime) % 2 == 0 )
                setActiveChannel(type_CCH);
            else
                setActiveChannel(type_SCH);

            // channel switching active
            nextChannelSwitch = new omnetpp::cMessage("Channel Switch");

            // add a little bit of offset between all vehicles, but no more than syncOffset
            omnetpp::simtime_t offset = dblrand() * par("syncOffset").doubleValue();
            scheduleAt(omnetpp::simTime() + timeToNextSwitch + offset, nextChannelSwitch);
        }
        else
        {
            // no channel switching
            nextChannelSwitch = 0;

            // channel is always on CCH
            setActiveChannel(type_CCH);
        }

        lastBusy = omnetpp::simTime();
        channelIdle(true);
    }
}


void Mac1609_4::finish()
{
    // update variables one last time
    for (auto &iter : myEDCA)
    {
        NumInternalContention += iter.second->NumInternalContention;
        NumBackoff += iter.second->NumBackoff;
        SlotsBackoff += iter.second->SlotsBackoff;
    }

    if(record_stat)
        record_MAC_stat_func();
}


void Mac1609_4::handleSelfMsg(omnetpp::cMessage* msg)
{
    // channel switch in alternating channel scheme
    if (msg == nextChannelSwitch)
    {
        ASSERT(useSCH);

        // schedule the next channel switch in 50ms
        scheduleAt(omnetpp::simTime() + SWITCHING_INTERVAL_11P, nextChannelSwitch);

        switch (activeChannel)
        {
        case type_CCH:
            EV << "CCH --> SCH" << std::endl;
            channelBusySelf(false);
            setActiveChannel(type_SCH);
            channelIdle(true);
            phy->changeListeningFrequency(frequency[mySCH]);
            break;

        case type_SCH:
            EV << "SCH --> CCH" << std::endl;
            channelBusySelf(false);
            setActiveChannel(type_CCH);
            channelIdle(true);
            phy->changeListeningFrequency(frequency[Channels::CCH]);
            break;
        }
    }
    else if (msg == nextMacEvent)
    {
        // we actually came to the point where we can send a packet
        channelBusySelf(true);

        // get the right WSM message from the queue
        WaveShortMessage* pktToSend = myEDCA[activeChannel]->initiateTransmit(lastIdle);

        // WSM message priority
        lastAC = mapPriority(pktToSend->getPriority());

        // encapsulated the WSM message
        Mac80211Pkt *macPkt = encapsMsg(pktToSend);

        // extract the control info from the MAC packet
        MacToPhyControlInfo *macCtl = dynamic_cast<MacToPhyControlInfo *>(macPkt->getControlInfo());
        assert(macCtl);

        omnetpp::simtime_t sendingDuration = RADIODELAY_11P + phy->getFrameDuration(macPkt->getBitLength(), macCtl->getBitrate(), macCtl->getMCS());

        EV << "Sending duration will be " << sendingDuration << std::endl;

        // if we are using 'continuous channel access' or we are using
        // 'alternating channel access' and we have enough time in the current
        // slot to send the frame
        if (!useSCH || timeLeftInSlot() > sendingDuration)
        {
            if (useSCH)
                EV << " Time in this slot left: " << timeLeftInSlot() << std::endl;

            // give time for the radio to be in Tx state before transmitting
            phy->setRadioState(Radio::TX);

            EV << "Sending MAC frame '" << macPkt->getFullName() << "' with priority " << lastAC << " to PHY layer." << std::endl;

            sendDelayed(macPkt, RADIODELAY_11P, lowerLayerOut);

            // note that we do not delete pktToSend message until PHY layer
            // reports successful transmission in handleLowerControl method
        }
        else
        {   // not enough time left now
            EV << "Too little Time left. This packet cannot be send in this slot. \n";

            NumTooLittleTime++;

            // revoke TXOP
            myEDCA[activeChannel]->revokeTxOPs();

            // delete the encapsulated message
            delete macPkt;

            channelIdle();

            // do nothing. contention will automatically start after channel switch
        }
    }
}


void Mac1609_4::handleUpperControl(omnetpp::cMessage* msg)
{
    assert(false);
}


// all messages from application layer are sent to this method!
void Mac1609_4::handleUpperMsg(omnetpp::cMessage* msg)
{
    // make sure that DSRCenabled is true on this node
    if(!ptrNode->par("DSRCenabled"))
        throw omnetpp::cRuntimeError("Cannot send msg %s: DSRCenabled parameter is false in %s", msg->getName(), ptrNode->getFullName());

    WaveShortMessage* thisMsg = dynamic_cast<WaveShortMessage*>(msg);
    if (thisMsg == NULL)
        throw omnetpp::cRuntimeError("Mac1609_4 only accepts messages of type WaveShortMessages!");

    t_access_category ac = mapPriority(thisMsg->getPriority());

    EV << "Received a message from upper layer for channel " << thisMsg->getChannelNumber() << " Access Category (Priority):  " << ac << std::endl;

    t_channel chan;

    // rewrite SCH channel to actual SCH the Mac1609_4 is set to
    if (thisMsg->getChannelNumber() == Channels::CCH)
        chan = type_CCH;
    else
    {
        ASSERT(useSCH);
        thisMsg->setChannelNumber(mySCH);
        chan = type_SCH;
    }

    int num = myEDCA[chan]->queuePacket(ac,thisMsg);

    // MAC frame was dropped in MAC
    if (num == -1)
    {
        NumDroppedFrames++;
        return;
    }

    EV << "sorted packet into queue of EDCA " << chan << " this packet is now at position: " << num << std::endl;

    if (chan == activeChannel)
        EV << "this packet is for the currently active channel \n";
    else
        EV << "this packet is NOT for the currently active channel \n";

    // if this packet is not at the front of a new queue we don't have to re-evaluate times
    if(num == 1 && chan == activeChannel)
    {
        if(idleChannel)
        {
            omnetpp::simtime_t nextEvent = myEDCA[chan]->startContent(lastIdle,guardActive());

            if (nextEvent != -1)
            {
                if ((!useSCH) || (nextEvent <= nextChannelSwitch->getArrivalTime()))
                {
                    if (nextMacEvent->isScheduled())
                        cancelEvent(nextMacEvent);

                    scheduleAt(nextEvent, nextMacEvent);

                    EV << "Updated nextMacEvent:" << nextMacEvent->getArrivalTime().raw() << std::endl;
                }
                else
                {
                    EV << "Too little time in this interval. Will not schedule nextMacEvent" << std::endl;

                    //it is possible that this queue has an txop. we have to revoke it
                    myEDCA[activeChannel]->revokeTxOPs();
                    NumTooLittleTime++;
                }
            }
            else
                cancelEvent(nextMacEvent);
        }
        else if(myEDCA[chan]->myQueues[ac].currentBackoff == 0)
            myEDCA[chan]->backoff(ac);
    }

    if(record_stat)
        record_MAC_stat_func();
}


void Mac1609_4::handleLowerMsg(omnetpp::cMessage* msg)
{
    Mac80211Pkt* macPkt = static_cast<Mac80211Pkt*>(msg);
    ASSERT(macPkt);

    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(macPkt->decapsulate());

    // pass information about received frame to the upper layers
    DeciderResult80211 *macRes = dynamic_cast<DeciderResult80211 *>(PhyToMacControlInfo::getDeciderResult(msg));
    ASSERT(macRes);

    DeciderResult80211 *res = new DeciderResult80211(*macRes);
    wsm->setControlInfo(new PhyToMacControlInfo(res));

    long dest = macPkt->getDestAddr();

    EV << "Received frame name= " << macPkt->getName() << ", myState=" << " src=" << macPkt->getSrcAddr() << " dst=" << macPkt->getDestAddr() << " myAddr=" << myMacAddress << std::endl;

    if (macPkt->getDestAddr() == myMacAddress)
    {
        EV << "Received a data packet addressed to me. \n";

        sendUp(wsm);
    }
    else if (dest == LAddress::L2BROADCAST())
    {
        sendUp(wsm);
    }
    else
    {
        EV << "Packet not for me, deleting... \n";
        delete wsm;
    }

    delete macPkt;
}


void Mac1609_4::handleLowerControl(omnetpp::cMessage* msg)
{
    if (msg->getKind() == MacToPhyInterface::TX_OVER)
    {
        EV << "Successfully transmitted a packet on " << lastAC << std::endl;

        phy->setRadioState(Radio::RX);

        // message was sent. update EDCA queue. go into post-transmit backoff and set cwCur to cwMin
        myEDCA[activeChannel]->postTransmit(lastAC);

        // channel just turned idle. don't set the channel to idle. the PHY layer decides, not us.

        if (guardActive())
            throw omnetpp::cRuntimeError("We shouldn't have sent a packet in guard!");
    }
    else if (msg->getKind() == MacToPhyInterface::RADIO_SWITCHING_OVER)
    {
        EV << "Phylayer said radio switching is done" << std::endl;
    }
    else if (msg->getKind() == Decider80211p::BITERROR)
    {
        EV << "A packet was not received due to biterrors" << std::endl;
    }
    else if (msg->getKind() == Decider80211p::COLLISION)
    {
        EV << "A packet was not received due to collision" << std::endl;

        emit(sigCollision, true);
    }
    else if (msg->getKind() == Decider80211p::RECWHILESEND)
    {
        EV << "A packet was not received because we were sending while receiving" << std::endl;
    }
    else if (msg->getKind() == BaseDecider::DROPPED)
    {
        EV << "Phylayer said packet was dropped" << std::endl;

        phy->setRadioState(Radio::RX);
    }
    else if (msg->getKind() == MacToPhyInterface::CHANNEL_BUSY)
    {
        channelBusy();
    }
    else if (msg->getKind() == MacToPhyInterface::CHANNEL_IDLE)
    {
        channelIdle();
    }
    else
    {
        EV << "Invalid control message type (type=NOTHING): name=" << msg->getName() << " modulesrc=" << msg->getSenderModule()->getFullPath() << "." << std::endl;
        assert(false);
    }

    if(record_stat)
        record_MAC_stat_func();

    delete msg;
}


/**
 * Decapsulates the network packet from the received MacPkt
 **/
omnetpp::cPacket* Mac1609_4::decapsMsg(MacPkt* msg)
{
    omnetpp::cPacket *m = msg->decapsulate();

    // Attaches a "control info" (MacToNetw) structure (object) to the message m.
    MacToNetwControlInfo::setControlInfo(m, msg->getSrcAddr());

    // delete the macPkt
    delete msg;

    coreEV << " message decapsulated " << std::endl;

    return m;
}


Mac80211Pkt* Mac1609_4::encapsMsg(WaveShortMessage *pktToSend)
{
    Mac80211Pkt* mac = new Mac80211Pkt(pktToSend->getName(), pktToSend->getKind());
    mac->setDestAddr(LAddress::L2BROADCAST());
    mac->setSrcAddr(myMacAddress);
    mac->encapsulate(pktToSend->dup());

    enum PHY_MCS mcs;
    double txPower_mW;
    uint64_t datarate;

    // extract the control info from the WSM message
    VENTOS::ApplToPhyControlInfo *controlInfo = dynamic_cast<VENTOS::ApplToPhyControlInfo *>(pktToSend->getControlInfo());

    if (controlInfo)
    {
        // get MCS from the control info
        // MCS specifies the modulation and coding scheme used for transmission
        mcs = (enum PHY_MCS)controlInfo->getMcs();

        // if MCS is not specified, just use the default one
        if (mcs != MCS_DEFAULT)
            datarate = getOFDMDatarate(mcs, BW_OFDM_10_MHZ);
        else
            datarate = bitrate;

        // get Tx power from the control info
        txPower_mW = controlInfo->getTxPower_mW();

        if (txPower_mW < 0)
            txPower_mW = txPower;
    }
    else
    {
        mcs = MCS_DEFAULT;
        txPower_mW = txPower;
        datarate = bitrate;
    }

    double freq = (activeChannel == type_CCH) ? frequency[Channels::CCH] : frequency[mySCH];

    // create a control info
    MacToPhyControlInfo* cinfo = new MacToPhyControlInfo();

    cinfo->setPower(txPower_mW);
    cinfo->setBitrate(datarate);
    cinfo->setMCS(mcs);
    cinfo->setFreq(freq);

    // attach the control info to the MAC
    mac->setControlInfo(cinfo);

    return mac;
}


void Mac1609_4::setActiveChannel(t_channel state)
{
    activeChannel = state;
    assert(state == type_CCH || (useSCH && state == type_SCH));
}


/* checks if guard is active */
bool Mac1609_4::guardActive() const
{
    if (!useSCH)
        return false;

    if (omnetpp::simTime().dbl() - nextChannelSwitch->getSendingTime() <= GUARD_INTERVAL_11P)
        return true;

    return false;
}


/* returns the time until the guard is over */
omnetpp::simtime_t Mac1609_4::timeLeftTillGuardOver() const
{
    ASSERT(useSCH);

    omnetpp::simtime_t sTime = omnetpp::simTime();

    if (sTime - nextChannelSwitch->getSendingTime() <= GUARD_INTERVAL_11P)
        return GUARD_INTERVAL_11P - (sTime - nextChannelSwitch->getSendingTime());
    else
        return 0;
}


/* returns the time left in this channel window */
omnetpp::simtime_t Mac1609_4::timeLeftInSlot() const
{
    ASSERT(useSCH);
    return nextChannelSwitch->getArrivalTime() - omnetpp::simTime();
}


/* Will change the Service Channel on which the mac layer is listening and sending */
void Mac1609_4::changeServiceChannel(int cN)
{
    ASSERT(useSCH);

    if (cN != Channels::SCH1 && cN != Channels::SCH2 && cN != Channels::SCH3 && cN != Channels::SCH4)
        throw omnetpp::cRuntimeError("This Service Channel doesnt exit: %d",cN);

    mySCH = cN;

    if (activeChannel == type_SCH)
    {
        //change to new channel immediately if we are in a SCH slot,
        //otherwise it will switch to the new SCH upon next channel switch
        phy->changeListeningFrequency(frequency[mySCH]);
    }
}


t_access_category Mac1609_4::mapPriority(int priority)
{
    // dummy mapping function
    switch (priority)
    {
    case 0:
        return AC_BK;
    case 1:
        return AC_BE;
    case 2:
        return AC_VI;
    case 3:
        return AC_VO;
    default:
        throw omnetpp::cRuntimeError("MacLayer received a packet with unknown priority");
    }

    return AC_VO;
}


void Mac1609_4::channelBusySelf(bool generateTxOp)
{
    //the channel turned busy because we're sending. we don't want our queues to go into backoff
    //internal contention is already handled in initiateTransmission

    if (!idleChannel)
        return;

    idleChannel = false;
    EV << "Channel turned busy: Switch or Self-Send" << std::endl;

    lastBusy = omnetpp::simTime();

    // channel turned busy
    if (nextMacEvent->isScheduled())
        cancelEvent(nextMacEvent);

    myEDCA[activeChannel]->stopContent(false, generateTxOp);

    emit(sigChannelBusy, true);
}


void Mac1609_4::channelBusy()
{
    if (!idleChannel) return;

    //the channel turned busy because someone else is sending
    idleChannel = false;
    EV << "Channel turned busy: External sender" << std::endl;
    lastBusy = omnetpp::simTime();

    //channel turned busy
    if (nextMacEvent->isScheduled())
        cancelEvent(nextMacEvent);

    myEDCA[activeChannel]->stopContent(true,false);

    emit(sigChannelBusy, true);
}


void Mac1609_4::channelIdle(bool afterSwitch)
{
    EV << "Channel turned idle: Switch: " << afterSwitch << std::endl;

    if (nextMacEvent->isScheduled())
    {
        //this rare case can happen when another node's time has such a big offset that the node sent a packet although we already changed the channel
        //the workaround is not trivial and requires a lot of changes to the phy and decider
        return;
        //throw omnetpp::cRuntimeError("channel turned idle but contention timer was scheduled!");
    }

    idleChannel = true;

    omnetpp::simtime_t delay = 0;

    // account for 1609.4 guards
    if (afterSwitch)
    {
        //  delay = GUARD_INTERVAL_11P;
    }

    if (useSCH)
        delay += timeLeftTillGuardOver();

    //channel turned idle! lets start contention!
    lastIdle = delay + omnetpp::simTime();
    TotalBusyTime += omnetpp::simTime() - lastBusy;

    //get next Event from current EDCA subsystem
    omnetpp::simtime_t nextEvent = myEDCA[activeChannel]->startContent(lastIdle,guardActive());
    if (nextEvent != -1)
    {
        if ((!useSCH) || (nextEvent < nextChannelSwitch->getArrivalTime()))
        {
            scheduleAt(nextEvent,nextMacEvent);

            EV << "next Event is at " << nextMacEvent->getArrivalTime().raw() << std::endl;
        }
        else
        {
            EV << "Too little time in this interval. will not schedule macEvent" << std::endl;

            NumTooLittleTime++;
            myEDCA[activeChannel]->revokeTxOPs();
        }
    }
    else
    {
        EV << "I don't have any new events in this EDCA sub system" << std::endl;
    }

    emit(sigChannelBusy, false);
}


bool Mac1609_4::isChannelSwitchingActive()
{
    return useSCH;
}


omnetpp::simtime_t Mac1609_4::getSwitchingInterval()
{
    return SWITCHING_INTERVAL_11P;
}


bool Mac1609_4::isCurrentChannelCCH()
{
    return (activeChannel == type_CCH);
}


void Mac1609_4::record_MAC_stat_func()
{
    VENTOS::MAC_stat_t entry = {};

    entry.last_stat_time = omnetpp::simTime().dbl();
    entry.NumDroppedFrames = NumDroppedFrames;
    entry.NumTooLittleTime = NumTooLittleTime;
    entry.NumInternalContention = NumInternalContention;
    entry.NumBackoff = NumBackoff;
    entry.SlotsBackoff = SlotsBackoff;
    entry.TotalBusyTime = TotalBusyTime.dbl();

    auto it = STAT->global_MAC_stat.find(myId);
    if(it == STAT->global_MAC_stat.end())
        STAT->global_MAC_stat[myId] = entry;
    else
        it->second = entry;
}

}

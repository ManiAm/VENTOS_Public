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
#include "DeciderResult80211.h"
#include "PhyToMacControlInfo.h"
#include "AddressingInterface.h"
#include "MacToNetwControlInfo.h"
#include "baseAppl/ApplToPhyControlInfo.h"

namespace Veins {

Define_Module(Veins::Mac1609_4);

void Mac1609_4::initialize(int stage)
{
    BaseLayer::initialize(stage);

    if(stage == 0)
    {
        // get handle to phy layer
        phy = FindModule<MacToPhyInterface*>::findSubModule(getParentModule());
        if (phy == NULL)
            throw omnetpp::cRuntimeError("Could not find a PHY module.");

        headerLength = par("headerLength");
        phyHeaderLength = phy->getPhyHeaderLength();

        hasPar("coreDebug") ? coreDebug = par("coreDebug").boolValue() : coreDebug = false;

        if (myMacAddr == LAddress::L2NULL())
        {
            // see if there is an addressing module available
            // otherwise use NIC modules id as MAC address
            AddressingInterface* addrScheme = FindModule<AddressingInterface*>::findSubModule(findHost());
            if(addrScheme)
                myMacAddr = addrScheme->myMacAddr(this);
            else
            {
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

        // get a pointer to the phy11p module
        phy11p = FindModule<Mac80211pToPhy11pInterface*>::findSubModule(getParentModule());
        assert(phy11p);

        // get a pointer to the Statistics module
        omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("statistics");
        STAT = static_cast<VENTOS::Statistics*>(module);
        ASSERT(STAT);

        record_stat = par("record_stat").boolValue();

        emulationActive = this->getParentModule()->par("emulationActive").boolValue();

        // this is required to circumvent double precision issues with constants from CONST80211p.h
        assert(omnetpp::simTime().getScaleExp() == -12);

        txPower = par("txPower").doubleValue();
        bitrate = par("bitrate").longValue();
        setParametersForBitrate(bitrate);

        // mac-adresses
        myMacAddress = intuniform(0,0xFFFFFFFE);
        myId = getParentModule()->getParentModule()->getFullName();

        headerLength = par("headerLength");

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
        myEDCA[type_CCH] = new EDCA(this, type_CCH, par("queueSize").longValue());
        myEDCA[type_CCH]->myId = myId;
        myEDCA[type_CCH]->myId.append(" CCH");
        // creating four queues
        myEDCA[type_CCH]->createQueue(2 /*AIFS*/, (((CWMIN_11P+1)/4)-1) /*CWmin*/, (((CWMIN_11P +1)/2)-1) /*CWmax*/, AC_VO);
        myEDCA[type_CCH]->createQueue(3 /*AIFS*/, (((CWMIN_11P+1)/2)-1) /*CWmin*/, CWMIN_11P /*CWmax*/, AC_VI);
        myEDCA[type_CCH]->createQueue(6 /*AIFS*/, CWMIN_11P /*CWmin*/, CWMAX_11P /*CWmax*/, AC_BE);
        myEDCA[type_CCH]->createQueue(9 /*AIFS*/, CWMIN_11P /*CWmin*/, CWMAX_11P /*CWmax*/, AC_BK);

        // EDCA for type_SCH
        myEDCA[type_SCH] = new EDCA(this, type_SCH, par("queueSize").longValue());
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
            //set the initial service channel
            switch (par("serviceChannel").longValue())
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

            if ((currenTime / switchingTime) % 2 == 0)
                setActiveChannel(type_CCH);
            else
                setActiveChannel(type_SCH);

            // channel switching active
            nextChannelSwitch = new omnetpp::cMessage("Channel Switch");

            // add a little bit of offset between all vehicles, but no more than syncOffset
            omnetpp::simtime_t offset = dblrand() * par("syncOffset").doubleValue();
            scheduleAt(omnetpp::simTime() + offset + timeToNextSwitch, nextChannelSwitch);
        }
        else
        {
            // no channel switching
            nextChannelSwitch = 0;
            setActiveChannel(type_CCH);
        }

        lastBusy = omnetpp::simTime();
        channelIdle(true);
    }
}


void Mac1609_4::finish()
{
    //clean up queues

    for (auto &iter : myEDCA)
    {
        statsNumInternalContention += iter.second->statsNumInternalContention;
        statsNumBackoff += iter.second->statsNumBackoff;
        statsSlotsBackoff += iter.second->statsSlotsBackoff;
        iter.second->cleanUp();
        delete iter.second;
    }

    myEDCA.clear();

    if (nextMacEvent->isScheduled())
        cancelAndDelete(nextMacEvent);
    else
        delete nextMacEvent;

    if (nextChannelSwitch && nextChannelSwitch->isScheduled())
        cancelAndDelete(nextChannelSwitch);
}


void Mac1609_4::handleSelfMsg(omnetpp::cMessage* msg)
{
    if (msg == nextChannelSwitch)
    {
        ASSERT(useSCH);

        scheduleAt(omnetpp::simTime() + SWITCHING_INTERVAL_11P, nextChannelSwitch);

        switch (activeChannel)
        {
        case type_CCH:
            EV << "CCH --> SCH" << std::endl;
            channelBusySelf(false);
            setActiveChannel(type_SCH);
            channelIdle(true);
            phy11p->changeListeningFrequency(frequency[mySCH]);
            break;

        case type_SCH:
            EV << "SCH --> CCH" << std::endl;
            channelBusySelf(false);
            setActiveChannel(type_CCH);
            channelIdle(true);
            phy11p->changeListeningFrequency(frequency[Channels::CCH]);
            break;
        }
        //schedule next channel switch in 50ms
    }
    else if (msg ==  nextMacEvent)
    {
        //we actually came to the point where we can send a packet
        channelBusySelf(true);
        WaveShortMessage* pktToSend = myEDCA[activeChannel]->initiateTransmit(lastIdle);

        lastAC = mapPriority(pktToSend->getPriority());

        EV << "MacEvent received. Trying to send packet with priority" << lastAC << std::endl;

        //send the packet
        Mac80211Pkt* mac = new Mac80211Pkt(pktToSend->getName(), pktToSend->getKind());
        mac->setDestAddr(LAddress::L2BROADCAST());
        mac->setSrcAddr(myMacAddress);
        mac->encapsulate(pktToSend->dup());

        enum PHY_MCS mcs;
        double txPower_mW;
        uint64_t datarate;
        VENTOS::ApplToPhyControlInfo *controlInfo = dynamic_cast<VENTOS::ApplToPhyControlInfo *>(pktToSend->getControlInfo());
        if (controlInfo)
        {
            //if MCS is not specified, just use the default one
            mcs = (enum PHY_MCS)controlInfo->getMcs();
            if (mcs != MCS_DEFAULT)
                datarate = getOfdmDatarate(mcs, BW_OFDM_10_MHZ);
            else
                datarate = bitrate;

            //apply the same principle to tx power
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

        omnetpp::simtime_t sendingDuration = RADIODELAY_11P + getFrameDuration(mac->getBitLength(), mcs);

        EV << "Sending duration will be" << sendingDuration << std::endl;

        if ((!useSCH) || (timeLeftInSlot() > sendingDuration))
        {
            if (useSCH)
                EV << " Time in this slot left: " << timeLeftInSlot() << std::endl;

            // give time for the radio to be in Tx state before transmitting
            phy->setRadioState(Radio::TX);

            double freq = (activeChannel == type_CCH) ? frequency[Channels::CCH] : frequency[mySCH];

            // create signal
            omnetpp::simtime_t duration = getFrameDuration(mac->getBitLength());
            Signal* s = createSignal(omnetpp::simTime()+RADIODELAY_11P, duration, txPower_mW, datarate, freq);

            // attach the signal to the mac message
            MacToPhyControlInfo* cinfo = new MacToPhyControlInfo(s);
            mac->setControlInfo(cinfo);

            EV << "Sending a Packet. Frequency " << freq << " Priority" << lastAC << std::endl;

            sendDelayed(mac, RADIODELAY_11P, lowerLayerOut);

            statsSentPackets++;
        }
        else
        {   //not enough time left now
            EV << "Too little Time left. This packet cannot be send in this slot. \n";
            statsNumTooLittleTime++;

            //revoke TXOP
            myEDCA[activeChannel]->revokeTxOPs();
            delete mac;
            channelIdle();
            //do nothing. contention will automatically start after channel switch
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
    // get a reference to this node
    omnetpp::cModule *thisNode = this->getParentModule()->getParentModule();
    // make sure that DSRCenabled is true on this node
    if(!thisNode->par("DSRCenabled"))
        throw omnetpp::cRuntimeError("Cannot send msg %s: DSRCenabled parameter is false in %s", msg->getName(), thisNode->getFullName());

    WaveShortMessage* thisMsg = dynamic_cast<WaveShortMessage*>(msg);

    if (thisMsg == NULL)
        throw omnetpp::cRuntimeError("WaveMac only accepts WaveShortMessages");

    if(!emulationActive)
    {
        // give time for the radio to be in Tx state before transmitting
        phy->setRadioState(Radio::TX);

        // create a mac frame
        Mac80211Pkt* mac = new Mac80211Pkt(thisMsg->getName(), thisMsg->getKind());
        mac->setDestAddr(LAddress::L2BROADCAST());
        mac->setSrcAddr(myMacAddress);
        mac->encapsulate(thisMsg);

        // and then send it
        sendDelayed(mac, RADIODELAY_11P, lowerLayerOut);

        statsSentPackets++;

        return;
    }

    t_access_category ac = mapPriority(thisMsg->getPriority());

    EV << "Received a message from upper layer for channel " << thisMsg->getChannelNumber() << " Access Category (Priority):  " << ac << std::endl;

    t_channel chan;

    //rewrite SCH channel to actual SCH the Mac1609_4 is set to
    if (thisMsg->getChannelNumber() == Channels::CCH)
        chan = type_CCH;
    else
    {
        ASSERT(useSCH);
        thisMsg->setChannelNumber(mySCH);
        chan = type_SCH;
    }

    int num = myEDCA[chan]->queuePacket(ac,thisMsg);

    //packet was dropped in Mac
    if (num == -1)
    {
        statsDroppedPackets++;
        return;
    }

    EV << "sorted packet into queue of EDCA " << chan << " this packet is now at position: " << num << std::endl;

    if (chan == activeChannel)
        EV << "this packet is for the currently active channel \n";
    else
        EV << "this packet is NOT for the currently active channel \n";

    //if this packet is not at the front of a new queue we don't have to re-evaluate times
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
                    statsNumTooLittleTime++;
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


void Mac1609_4::handleLowerControl(omnetpp::cMessage* msg)
{
    if (msg->getKind() == MacToPhyInterface::TX_OVER)
    {
        EV << "Successfully transmitted a packet on " << lastAC << std::endl;

        phy->setRadioState(Radio::RX);

        if(emulationActive)
        {
            // message was sent. update EDCA queue. go into post-transmit backoff and set cwCur to cwMin
            myEDCA[activeChannel]->postTransmit(lastAC);

            // channel just turned idle. don't set the chan to idle. the PHY layer decides, not us.

            if (guardActive())
                throw omnetpp::cRuntimeError("We shouldn't have sent a packet in guard!");
        }
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
    else if (msg->getKind() == BaseDecider::PACKET_DROPPED)
    {
        EV << "Phylayer said packet was dropped" << std::endl;

        phy->setRadioState(Radio::RX);
    }
    else if (msg->getKind() == Mac80211pToPhy11pInterface::CHANNEL_BUSY)
    {
        channelBusy();
    }
    else if (msg->getKind() == Mac80211pToPhy11pInterface::CHANNEL_IDLE)
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


void Mac1609_4::registerInterface()
{

}


/**
 * Decapsulates the network packet from the received MacPkt
 **/
omnetpp::cPacket* Mac1609_4::decapsMsg(MacPkt* msg)
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
MacPkt* Mac1609_4::encapsMsg(omnetpp::cPacket *netwPkt)
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


void Mac1609_4::setActiveChannel(t_channel state)
{
    activeChannel = state;
    assert(state == type_CCH || (useSCH && state == type_SCH));
}


Signal* Mac1609_4::createSignal(omnetpp::simtime_t start, omnetpp::simtime_t length, double power, uint64_t bitrate, double frequency)
{
    omnetpp::simtime_t end = start + length;
    //create signal with start at current simtime and passed length
    Signal* s = new Signal(start, length);

    //create and set tx power mapping
    ConstMapping* txPowerMapping = createSingleFrequencyMapping(start, end, frequency, 5.0e6, power);
    s->setTransmissionPower(txPowerMapping);

    Mapping* bitrateMapping = MappingUtils::createMapping(DimensionSet::timeDomain(), Mapping::STEPS);

    Argument pos(start);
    bitrateMapping->setValue(pos, bitrate);

    pos.setTime(phyHeaderLength / bitrate);
    bitrateMapping->setValue(pos, bitrate);

    s->setBitrate(bitrateMapping);

    return s;
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
        //change to new chan immediately if we are in a SCH slot,
        //otherwise it will switch to the new SCH upon next channel switch
        phy11p->changeListeningFrequency(frequency[mySCH]);
    }
}


void Mac1609_4::setTxPower(double txPower_mW)
{
    txPower = txPower_mW;
}


void Mac1609_4::setMCS(enum PHY_MCS mcs)
{
    ASSERT2(mcs != MCS_DEFAULT, "invalid MCS selected");

    bitrate = getOfdmDatarate(mcs, BW_OFDM_10_MHZ);
    setParametersForBitrate(bitrate);
}


void Mac1609_4::setCCAThreshold(double ccaThreshold_dBm)
{
    phy11p->setCCAThreshold(ccaThreshold_dBm);
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

        statsReceivedPackets++;
        sendUp(wsm);
    }
    else if (dest == LAddress::L2BROADCAST())
    {
        statsReceivedBroadcasts++;
        sendUp(wsm);
    }
    else
    {
        EV << "Packet not for me, deleting... \n";
        delete wsm;
    }

    delete macPkt;
}


t_access_category Mac1609_4::mapPriority(int prio)
{
    //dummy mapping function
    switch (prio)
    {
    case 0: return AC_BK;
    case 1: return AC_BE;
    case 2: return AC_VI;
    case 3: return AC_VO;
    default: throw omnetpp::cRuntimeError("MacLayer received a packet with unknown priority"); break;
    }

    return AC_VO;
}


void Mac1609_4::channelBusySelf(bool generateTxOp)
{
    //the channel turned busy because we're sending. we don't want our queues to go into backoff
    //internal contention is already handled in initiateTransmission

    if (!idleChannel) return;
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

    //account for 1609.4 guards
    if (afterSwitch)
    {
        //  delay = GUARD_INTERVAL_11P;
    }

    if (useSCH)
        delay += timeLeftTillGuardOver();

    //channel turned idle! lets start contention!
    lastIdle = delay + omnetpp::simTime();
    statsTotalBusyTime += omnetpp::simTime() - lastBusy;

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

            statsNumTooLittleTime++;
            myEDCA[activeChannel]->revokeTxOPs();
        }
    }
    else
    {
        EV << "I don't have any new events in this EDCA sub system" << std::endl;
    }

    emit(sigChannelBusy, false);
}


void Mac1609_4::setParametersForBitrate(uint64_t bitrate)
{
    for (unsigned int i = 0; i < NUM_BITRATES_80211P; i++)
    {
        if (bitrate == BITRATES_80211P[i])
        {
            n_dbps = N_DBPS_80211P[i];
            return;
        }
    }

    throw omnetpp::cRuntimeError("Chosen Bitrate is not valid for 802.11p: Valid rates are: 3Mbps, 4.5Mbps, 6Mbps, 9Mbps, 12Mbps, 18Mbps, 24Mbps and 27Mbps. Please adjust your omnetpp.ini file accordingly.");
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


omnetpp::simtime_t Mac1609_4::getFrameDuration(int payloadLengthBits, enum PHY_MCS mcs) const
{
    omnetpp::simtime_t duration;

    if (mcs == MCS_DEFAULT)
    {
        // calculate frame duration according to Equation (17-29) of the IEEE 802.11-2007 standard
        duration = PHY_HDR_PREAMBLE_DURATION + PHY_HDR_PLCPSIGNAL_DURATION + T_SYM_80211P * ceil( (16 + payloadLengthBits + 6)/(n_dbps) );
    }
    else
    {
        uint32_t ndbps = getNDBPS(mcs);
        duration = PHY_HDR_PREAMBLE_DURATION + PHY_HDR_PLCPSIGNAL_DURATION + T_SYM_80211P * ceil( (16 + payloadLengthBits + 6)/(ndbps) );
    }

    return duration;
}


Signal* Mac1609_4::createSimpleSignal(omnetpp::simtime_t_cref start, omnetpp::simtime_t_cref length, double power, double bitrate)
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


Mapping* Mac1609_4::createConstantMapping(omnetpp::simtime_t_cref start, omnetpp::simtime_t_cref end, Argument::mapped_type_cref value)
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


Mapping* Mac1609_4::createRectangleMapping(omnetpp::simtime_t_cref start, omnetpp::simtime_t_cref end, Argument::mapped_type_cref value)
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


ConstMapping* Mac1609_4::createSingleFrequencyMapping(omnetpp::simtime_t_cref start,
        omnetpp::simtime_t_cref end,
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


BaseConnectionManager* Mac1609_4::getConnectionManager()
{
    cModule* nic = getParentModule();
    return ChannelAccess::getConnectionManager(nic);
}


const LAddress::L2Type& Mac1609_4::getUpperDestinationFromControlInfo(const cObject *const pCtrlInfo)
{
    return NetwToMacControlInfo::getDestFromControlInfo(pCtrlInfo);
}


/**
 * Attaches a "control info" (MacToNetw) structure (object) to the message pMsg.
 */
omnetpp::cObject *const Mac1609_4::setUpControlInfo(omnetpp::cMessage *const pMsg, const LAddress::L2Type& pSrcAddr)
{
    return MacToNetwControlInfo::setControlInfo(pMsg, pSrcAddr);
}


/**
 * Attaches a "control info" (MacToPhy) structure (object) to the message pMsg.
 */
omnetpp::cObject *const Mac1609_4::setDownControlInfo(omnetpp::cMessage *const pMsg, Signal *const pSignal)
{
    return MacToPhyControlInfo::setControlInfo(pMsg, pSignal);
}


void Mac1609_4::record_MAC_stat_func()
{
    VENTOS::MAC_stat_t entry = {};

    entry.last_stat_time = omnetpp::simTime().dbl();
    entry.statsDroppedPackets = statsDroppedPackets;
    entry.statsNumTooLittleTime = statsNumTooLittleTime;
    entry.statsNumInternalContention = statsNumInternalContention;
    entry.statsNumBackoff = statsNumBackoff;
    entry.statsSlotsBackoff = statsSlotsBackoff;
    entry.statsTotalBusyTime = statsTotalBusyTime.dbl();
    entry.statsSentPackets = statsSentPackets;
    entry.statsReceivedPackets = statsReceivedPackets;
    entry.statsReceivedBroadcasts = statsReceivedBroadcasts;

    auto it = STAT->global_MAC_stat.find(myId);
    if(it == STAT->global_MAC_stat.end())
        STAT->global_MAC_stat[myId] = entry;
    else
        it->second = entry;
}

}

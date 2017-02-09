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

#include <iterator>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "Mac1609_4.h"
#include "DeciderResult80211.h"
#include "PhyToMacControlInfo.h"
#include "PhyControlMessage_m.h"

namespace Veins {

Define_Module(Veins::Mac1609_4);

void Mac1609_4::initialize(int stage)
{
    BaseMacLayer::initialize(stage);

    if (stage == 0)
    {
        phy11p = FindModule<Mac80211pToPhy11pInterface*>::findSubModule(getParentModule());
        assert(phy11p);

        //this is required to circumvent double precision issues with constants from CONST80211p.h
        assert(omnetpp::simTime().getScaleExp() == -12);

        txPower = par("txPower").doubleValue();
        bitrate = par("bitrate").longValue();
        setParametersForBitrate(bitrate);

        //mac-adresses
        myMacAddress = intuniform(0,0xFFFFFFFE);
        myId = getParentModule()->getParentModule()->getFullPath();

        headerLength = par("headerLength");

        nextMacEvent = new omnetpp::cMessage("next Mac Event");

        sigChannelBusy = registerSignal("sigChannelBusy");
        sigCollision = registerSignal("sigCollision");

        //create frequency mappings
        frequency[Channels::CRIT_SOL] = 5.86e9;
        frequency[Channels::SCH1] = 5.87e9;
        frequency[Channels::SCH2] = 5.88e9;
        frequency[Channels::CCH] = 5.89e9;
        frequency[Channels::SCH3] = 5.90e9;
        frequency[Channels::SCH4] = 5.91e9;
        frequency[Channels::HPPS] = 5.92e9;

        //create two EDCA systems

        myEDCA[type_CCH] = new EDCA(this, type_CCH, par("queueSize").longValue());
        myEDCA[type_CCH]->myId = myId;
        myEDCA[type_CCH]->myId.append(" CCH");
        myEDCA[type_CCH]->createQueue(2,(((CWMIN_11P+1)/4)-1),(((CWMIN_11P +1)/2)-1),AC_VO);
        myEDCA[type_CCH]->createQueue(3,(((CWMIN_11P+1)/2)-1),CWMIN_11P,AC_VI);
        myEDCA[type_CCH]->createQueue(6,CWMIN_11P,CWMAX_11P,AC_BE);
        myEDCA[type_CCH]->createQueue(9,CWMIN_11P,CWMAX_11P,AC_BK);

        myEDCA[type_SCH] = new EDCA(this, type_SCH, par("queueSize").longValue());
        myEDCA[type_SCH]->myId = myId;
        myEDCA[type_SCH]->myId.append(" SCH");
        myEDCA[type_SCH]->createQueue(2,(((CWMIN_11P+1)/4)-1),(((CWMIN_11P +1)/2)-1),AC_VO);
        myEDCA[type_SCH]->createQueue(3,(((CWMIN_11P+1)/2)-1),CWMIN_11P,AC_VI);
        myEDCA[type_SCH]->createQueue(6,CWMIN_11P,CWMAX_11P,AC_BE);
        myEDCA[type_SCH]->createQueue(9,CWMIN_11P,CWMAX_11P,AC_BK);

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

        record_stat = par("record_stat").boolValue();

        // get a pointer to the TraCI module
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<VENTOS::TraCI_Commands *>(module);
        ASSERT(TraCI);

        // get a pointer to the Statistics module
        module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("statistics");
        STAT = static_cast<VENTOS::Statistics *>(module);
        ASSERT(STAT);
    }
}


void Mac1609_4::finish()
{
    //clean up queues.

    for (std::map<t_channel,EDCA*>::iterator iter = myEDCA.begin(); iter != myEDCA.end(); iter++)
    {
        statsNumInternalContention += iter->second->statsNumInternalContention;
        statsNumBackoff += iter->second->statsNumBackoff;
        statsSlotsBackoff += iter->second->statsSlotsBackoff;
        iter->second->cleanUp();
        delete iter->second;
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
        PhyControlMessage *controlInfo = dynamic_cast<PhyControlMessage *>(pktToSend->getControlInfo());
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
            if (useSCH) EV << " Time in this slot left: " << timeLeftInSlot() << std::endl;
            // give time for the radio to be in Tx state before transmitting
            phy->setRadioState(Radio::TX);

            double freq = (activeChannel == type_CCH) ? frequency[Channels::CCH] : frequency[mySCH];

            attachSignal(mac, omnetpp::simTime()+RADIODELAY_11P, freq, datarate, txPower_mW);
            MacToPhyControlInfo* phyInfo = dynamic_cast<MacToPhyControlInfo*>(mac->getControlInfo());
            assert(phyInfo);
            EV << "Sending a Packet. Frequency " << freq << " Priority" << lastAC << std::endl;
            sendDelayed(mac, RADIODELAY_11P, lowerLayerOut);
            statsSentPackets++;
        }
        else
        {   //not enough time left now
            EV << "Too little Time left. This packet cannot be send in this slot." << std::endl;
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
    WaveShortMessage* thisMsg = dynamic_cast<WaveShortMessage*>(msg);

    if (thisMsg == NULL)
        throw omnetpp::cRuntimeError("WaveMac only accepts WaveShortMessages");

    t_access_category ac = mapPriority(thisMsg->getPriority());

    EV << "Received a message from upper layer for channel "
            << thisMsg->getChannelNumber() << " Access Category (Priority):  "
            << ac << std::endl;

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

    //if this packet is not at the front of a new queue we don't have to re-evaluate times
    EV << "sorted packet into queue of EDCA " << chan << " this packet is now at position: " << num << std::endl;

    if (chan == activeChannel)
        EV << "this packet is for the currently active channel \n";
    else
        EV << "this packet is NOT for the currently active channel \n";

    if (num == 1 && idleChannel && chan == activeChannel)
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
        {
            cancelEvent(nextMacEvent);
        }
    }

    if (num == 1 && !idleChannel && myEDCA[chan]->myQueues[ac].currentBackoff == 0 && chan == activeChannel)
        myEDCA[chan]->backoff(ac);

    if(record_stat)
        record_MAC_stat_func();
}

void Mac1609_4::handleLowerControl(omnetpp::cMessage* msg)
{
    if (msg->getKind() == MacToPhyInterface::TX_OVER)
    {
        EV << "Successfully transmitted a packet on " << lastAC << std::endl;

        phy->setRadioState(Radio::RX);

        //message was sent
        //update EDCA queue. go into post-transmit backoff and set cwCur to cwMin
        myEDCA[activeChannel]->postTransmit(lastAC);
        //channel just turned idle.
        //don't set the chan to idle. the PHY layer decides, not us.

        if (guardActive())
            throw omnetpp::cRuntimeError("We shouldnt have sent a packet in guard!");
    }
    else if (msg->getKind() == Mac80211pToPhy11pInterface::CHANNEL_BUSY)
    {
        channelBusy();
    }
    else if (msg->getKind() == Mac80211pToPhy11pInterface::CHANNEL_IDLE)
    {
        channelIdle();
    }
    else if (msg->getKind() == Decider80211p::BITERROR || msg->getKind() == Decider80211p::COLLISION)
    {
        statsSNIRLostPackets++;
        EV << "A packet was not received due to biterrors" << std::endl;
    }
    else if (msg->getKind() == Decider80211p::RECWHILESEND)
    {
        statsTXRXLostPackets++;
        EV << "A packet was not received because we were sending while receiving" << std::endl;
    }
    else if (msg->getKind() == MacToPhyInterface::RADIO_SWITCHING_OVER)
    {
        EV << "Phylayer said radio switching is done" << std::endl;
    }
    else if (msg->getKind() == BaseDecider::PACKET_DROPPED)
    {
        phy->setRadioState(Radio::RX);
        EV << "Phylayer said packet was dropped" << std::endl;
    }
    else
    {
        EV << "Invalid control message type (type=NOTHING) : name=" << msg->getName() << " modulesrc=" << msg->getSenderModule()->getFullPath() << "." << std::endl;
        assert(false);
    }

    if (msg->getKind() == Decider80211p::COLLISION)
        emit(sigCollision, true);

    delete msg;

    if(record_stat)
        record_MAC_stat_func();
}

void Mac1609_4::setActiveChannel(t_channel state)
{
    activeChannel = state;
    assert(state == type_CCH || (useSCH && state == type_SCH));
}


void Mac1609_4::attachSignal(Mac80211Pkt* mac, omnetpp::simtime_t startTime, double frequency, uint64_t datarate, double txPower_mW)
{
    omnetpp::simtime_t duration = getFrameDuration(mac->getBitLength());

    Signal* s = createSignal(startTime, duration, txPower_mW, datarate, frequency);
    MacToPhyControlInfo* cinfo = new MacToPhyControlInfo(s);

    mac->setControlInfo(cinfo);
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
    {
        return GUARD_INTERVAL_11P - (sTime - nextChannelSwitch->getSendingTime());
    }
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

    WaveShortMessage*  wsm =  dynamic_cast<WaveShortMessage*>(macPkt->decapsulate());

    //pass information about received frame to the upper layers
    DeciderResult80211 *macRes = dynamic_cast<DeciderResult80211 *>(PhyToMacControlInfo::getDeciderResult(msg));
    ASSERT(macRes);

    DeciderResult80211 *res = new DeciderResult80211(*macRes);
    wsm->setControlInfo(new PhyToMacControlInfo(res));

    long dest = macPkt->getDestAddr();

    EV << "Received frame name= " << macPkt->getName()
	                                                                                        << ", myState=" << " src=" << macPkt->getSrcAddr()
	                                                                                        << " dst=" << macPkt->getDestAddr() << " myAddr="
	                                                                                        << myMacAddress << std::endl;

    if (macPkt->getDestAddr() == myMacAddress)
    {
        EV << "Received a data packet addressed to me." << std::endl;
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
        EV << "Packet not for me, deleting..." << std::endl;
        delete wsm;
    }

    delete macPkt;
}

int Mac1609_4::EDCA::queuePacket(t_access_category ac,WaveShortMessage* msg)
{

    if (maxQueueSize && myQueues[ac].queue.size() >= maxQueueSize)
    {
        delete msg;
        return -1;
    }

    myQueues[ac].queue.push(msg);
    return myQueues[ac].queue.size();
}

int Mac1609_4::EDCA::createQueue(int aifsn, int cwMin, int cwMax,t_access_category ac)
{
    if (myQueues.find(ac) != myQueues.end())
        throw omnetpp::cRuntimeError("You can only add one queue per Access Category per EDCA subsystem");

    EDCAQueue newQueue(aifsn,cwMin,cwMax,ac);
    myQueues[ac] = newQueue;

    return ++numQueues;
}

Mac1609_4::t_access_category Mac1609_4::mapPriority(int prio)
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

WaveShortMessage* Mac1609_4::EDCA::initiateTransmit(omnetpp::simtime_t lastIdle)
{
    EV_STATICCONTEXT

    //iterate through the queues to return the packet we want to send
    WaveShortMessage* pktToSend = NULL;

    omnetpp::simtime_t idleTime = omnetpp::simTime() - lastIdle;

    EV << "Initiating transmit at " << omnetpp::simTime() << ". I've been idle since " << idleTime << std::endl;

    for (std::map<t_access_category, EDCAQueue>::iterator iter = myQueues.begin(); iter != myQueues.end(); iter++)
    {
        if (iter->second.queue.size() != 0)
        {
            if (idleTime >= iter->second.aifsn* SLOTLENGTH_11P + SIFS_11P && iter->second.txOP == true)
            {
                EV << "Queue " << iter->first << " is ready to send!" << std::endl;

                iter->second.txOP = false;
                //this queue is ready to send
                if (pktToSend == NULL)
                {
                    pktToSend = iter->second.queue.front();
                }
                else
                {
                    //there was already another packet ready. we have to go increase cw and go into backoff. It's called internal contention and its wonderful

                    statsNumInternalContention++;
                    iter->second.cwCur = std::min(iter->second.cwMax,(iter->second.cwCur+1)*2-1);
                    iter->second.currentBackoff = owner->intuniform(0,iter->second.cwCur);
                    EV << "Internal contention for queue " << iter->first  << " : "<< iter->second.currentBackoff << ". Increase cwCur to " << iter->second.cwCur << std::endl;
                }
            }
        }
    }

    if (pktToSend == NULL)
        throw omnetpp::cRuntimeError("No packet was ready");

    return pktToSend;
}

omnetpp::simtime_t Mac1609_4::EDCA::startContent(omnetpp::simtime_t idleSince, bool guardActive)
{
    EV_STATICCONTEXT

    EV << "Restarting contention." << std::endl;

    omnetpp::simtime_t nextEvent = -1;

    omnetpp::simtime_t idleTime = omnetpp::SimTime().setRaw(std::max((int64_t)0,(omnetpp::simTime() - idleSince).raw()));;

    lastStart = idleSince;

    EV << "Channel is already idle for:" << idleTime << " since " << idleSince << std::endl;

    //this returns the nearest possible event in this EDCA subsystem after a busy channel

    for (std::map<t_access_category, EDCAQueue>::iterator iter = myQueues.begin(); iter != myQueues.end(); iter++)
    {
        if (iter->second.queue.size() != 0)
        {
            /* 1609_4 says that when attempting to send (backoff == 0) when guard is active, a random backoff is invoked */

            if (guardActive == true && iter->second.currentBackoff == 0)
            {
                //cw is not increased
                iter->second.currentBackoff = owner->intuniform(0,iter->second.cwCur);
                statsNumBackoff++;
            }

            omnetpp::simtime_t DIFS = iter->second.aifsn * SLOTLENGTH_11P + SIFS_11P;

            //the next possible time to send can be in the past if the channel was idle for a long time, meaning we COULD have sent earlier if we had a packet
            omnetpp::simtime_t possibleNextEvent = DIFS + iter->second.currentBackoff * SLOTLENGTH_11P;


            EV << "Waiting Time for Queue " << iter->first <<  ":" << possibleNextEvent << "=" << iter->second.aifsn << " * "  << SLOTLENGTH_11P << " + " << SIFS_11P << "+" << iter->second.currentBackoff << "*" << SLOTLENGTH_11P << "; Idle time: " << idleTime << std::endl;

            if (idleTime > possibleNextEvent)
            {
                EV << "Could have already send if we had it earlier" << std::endl;
                //we could have already sent. round up to next boundary
                omnetpp::simtime_t base = idleSince + DIFS;
                possibleNextEvent =  omnetpp::simTime() - omnetpp::simtime_t().setRaw((omnetpp::simTime() - base).raw() % SLOTLENGTH_11P.raw()) + SLOTLENGTH_11P;
            }
            else
            {
                //we are gonna send in the future
                EV << "Sending in the future" << std::endl;
                possibleNextEvent =  idleSince + possibleNextEvent;
            }

            nextEvent == -1? nextEvent =  possibleNextEvent : nextEvent = std::min(nextEvent,possibleNextEvent);
        }
    }

    return nextEvent;
}

void Mac1609_4::EDCA::stopContent(bool allowBackoff, bool generateTxOp)
{
    EV_STATICCONTEXT

    //update all Queues

    EV << "Stopping Contention at " << omnetpp::simTime().raw() << std::endl;

    omnetpp::simtime_t passedTime = omnetpp::simTime() - lastStart;

    EV << "Channel was idle for " << passedTime << std::endl;

    lastStart = -1; //indicate that there was no last start

    for (std::map<t_access_category, EDCAQueue>::iterator iter = myQueues.begin(); iter != myQueues.end(); iter++)
    {
        if (iter->second.currentBackoff != 0 || iter->second.queue.size() != 0)
        {
            //check how many slots we already waited until the chan became busy

            int oldBackoff = iter->second.currentBackoff;

            std::string info;
            if (passedTime < iter->second.aifsn * SLOTLENGTH_11P + SIFS_11P)
            {
                //we didnt even make it one DIFS :(
                info.append(" No DIFS");
            }
            else
            {
                //decrease the backoff by one because we made it longer than one DIFS
                iter->second.currentBackoff--;

                //check how many slots we waited after the first DIFS
                int passedSlots = (int)((passedTime - omnetpp::SimTime(iter->second.aifsn * SLOTLENGTH_11P + SIFS_11P)) / SLOTLENGTH_11P);

                EV << "Passed slots after DIFS: " << passedSlots << std::endl;


                if (iter->second.queue.size() == 0)
                {
                    //this can be below 0 because of post transmit backoff -> backoff on empty queues will not generate macevents,
                    //we dont want to generate a txOP for empty queues
                    iter->second.currentBackoff -= std::min(iter->second.currentBackoff,passedSlots);
                    info.append(" PostCommit Over");
                }
                else
                {
                    iter->second.currentBackoff -= passedSlots;
                    if (iter->second.currentBackoff <= -1)
                    {
                        if (generateTxOp)
                        {
                            iter->second.txOP = true; info.append(" TXOP");
                        }
                        //else: this packet couldnt be sent because there was too little time. we could have generated a txop, but the channel switched
                        iter->second.currentBackoff = 0;
                    }

                }
            }

            EV << "Updating backoff for Queue " << iter->first << ": " << oldBackoff << " -> " << iter->second.currentBackoff << info <<std::endl;
        }
    }
}

void Mac1609_4::EDCA::backoff(t_access_category ac)
{
    EV_STATICCONTEXT

    myQueues[ac].currentBackoff = owner->intuniform(0,myQueues[ac].cwCur);
    statsSlotsBackoff += myQueues[ac].currentBackoff;
    statsNumBackoff++;
    EV << "Going into Backoff because channel was busy when new packet arrived from upperLayer" << std::endl;
}

void Mac1609_4::EDCA::postTransmit(t_access_category ac)
{
    EV_STATICCONTEXT

    delete myQueues[ac].queue.front();
    myQueues[ac].queue.pop();
    myQueues[ac].cwCur = myQueues[ac].cwMin;
    //post transmit backoff
    myQueues[ac].currentBackoff = owner->intuniform(0,myQueues[ac].cwCur);
    statsSlotsBackoff += myQueues[ac].currentBackoff;
    statsNumBackoff++;
    EV << "Queue " << ac << " will go into post-transmit backoff for " << myQueues[ac].currentBackoff << " slots" << std::endl;
}

void Mac1609_4::EDCA::cleanUp()
{
    for (std::map<t_access_category, EDCAQueue>::iterator iter = myQueues.begin(); iter != myQueues.end(); iter++)
    {
        while (iter->second.queue.size() != 0)
        {
            delete iter->second.queue.front();
            iter->second.queue.pop();
        }
    }

    myQueues.clear();
}

void Mac1609_4::EDCA::revokeTxOPs()
{
    for (std::map<t_access_category, EDCAQueue>::iterator iter = myQueues.begin(); iter != myQueues.end(); iter++)
    {
        if (iter->second.txOP == true)
        {
            iter->second.txOP = false;
            iter->second.currentBackoff = 0;
        }
    }
}

void Mac1609_4::channelBusySelf(bool generateTxOp)
{
    //the channel turned busy because we're sending. we don't want our queues to go into backoff
    //internal contention is already handled in initiateTransmission

    if (!idleChannel) return;
    idleChannel = false;
    EV << "Channel turned busy: Switch or Self-Send" << std::endl;

    lastBusy = omnetpp::simTime();

    //channel turned busy
    if (nextMacEvent->isScheduled() == true)
    {
        cancelEvent(nextMacEvent);
    }
    else
    {
        //the edca subsystem was not doing anything anyway.
    }

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
    if (nextMacEvent->isScheduled() == true)
    {
        cancelEvent(nextMacEvent);
    }
    else
    {
        //the edca subsystem was not doing anything anyway.
    }

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
        //	delay = GUARD_INTERVAL_11P;
    }

    if (useSCH)
    {
        delay += timeLeftTillGuardOver();
    }

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
    return (activeChannel ==  type_CCH);
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


void Mac1609_4::record_MAC_stat_func()
{
    VENTOS::MAC_stat_entry_t entry = {};

    entry.last_stat_time = omnetpp::simTime().dbl();
    entry.statsDroppedPackets = statsDroppedPackets;
    entry.statsNumTooLittleTime = statsNumTooLittleTime;
    entry.statsNumInternalContention = statsNumInternalContention;
    entry.statsNumBackoff = statsNumBackoff;
    entry.statsSlotsBackoff = statsSlotsBackoff;
    entry.statsTotalBusyTime = statsTotalBusyTime.dbl();
    entry.statsSentPackets = statsSentPackets;
    entry.statsSNIRLostPackets = statsSNIRLostPackets;
    entry.statsTXRXLostPackets = statsTXRXLostPackets;
    entry.statsReceivedPackets = statsReceivedPackets;
    entry.statsReceivedBroadcasts = statsReceivedBroadcasts;

    auto it = STAT->global_MAC_stat.find(myId);
    if(it == STAT->global_MAC_stat.end())
        STAT->global_MAC_stat[myId] = entry;
    else
        it->second = entry;
}

}

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

#include "Mac1609_4_EDCA.h"

namespace Veins {


void EDCA::createQueue(int aifsn, int cwMin, int cwMax, t_access_category ac)
{
    if (myQueues.find(ac) != myQueues.end())
        throw omnetpp::cRuntimeError("You can only add one queue per Access Category per EDCA subsystem");

    EDCAQueue_t newQueue(aifsn, cwMin, cwMax, ac);
    myQueues[ac] = newQueue;
}


int EDCA::queuePacket(t_access_category ac,WaveShortMessage* msg)
{
    if (maxQueueSize && myQueues[ac].queue.size() >= maxQueueSize)
    {
        delete msg;
        return -1;
    }

    myQueues[ac].queue.push(msg);
    return myQueues[ac].queue.size();
}


omnetpp::simtime_t EDCA::startContent(omnetpp::simtime_t idleSince, bool guardActive)
{
    EV_STATICCONTEXT

    EV << "Restarting contention. \n";

    omnetpp::simtime_t nextEvent = -1;

    omnetpp::simtime_t idleTime = omnetpp::SimTime().setRaw(std::max((int64_t)0, (omnetpp::simTime() - idleSince).raw()));;

    lastStart = idleSince;

    EV << "Channel is already idle for:" << idleTime << " since " << idleSince << std::endl;

    //this returns the nearest possible event in this EDCA subsystem after a busy channel

    for (auto &iter : myQueues)
    {
        if (iter.second.queue.size() != 0)
        {
            /* 1609_4 says that when attempting to send (backoff == 0) when guard is active, a random backoff is invoked */

            if (guardActive && iter.second.currentBackoff == 0)
            {
                //cw is not increased
                iter.second.currentBackoff = owner->intuniform(0, iter.second.cwCur);
                NumBackoff++;
            }

            omnetpp::simtime_t DIFS = iter.second.aifsn * SLOTLENGTH_11P + SIFS_11P;

            // the next possible time to send can be in the past if the channel was idle for
            // a long time, meaning we COULD have sent earlier if we had a packet
            omnetpp::simtime_t possibleNextEvent = DIFS + iter.second.currentBackoff * SLOTLENGTH_11P;

            EV << "Waiting Time for Queue " << iter.first <<  ":" << possibleNextEvent << "=" << iter.second.aifsn << " * "  << SLOTLENGTH_11P << " + " << SIFS_11P << "+" << iter.second.currentBackoff << "*" << SLOTLENGTH_11P << "; Idle time: " << idleTime << std::endl;

            if (idleTime > possibleNextEvent)
            {
                EV << "Could have already send if we had it earlier" << std::endl;

                // we could have already sent. round up to next boundary
                omnetpp::simtime_t base = idleSince + DIFS;
                possibleNextEvent =  omnetpp::simTime() - omnetpp::simtime_t().setRaw((omnetpp::simTime() - base).raw() % SLOTLENGTH_11P.raw()) + SLOTLENGTH_11P;
            }
            else
            {
                // we are going to send in the future
                EV << "Sending in the future \n";
                possibleNextEvent =  idleSince + possibleNextEvent;
            }

            nextEvent = (nextEvent == -1) ? possibleNextEvent : std::min(nextEvent, possibleNextEvent);
        }
    }

    return nextEvent;
}


void EDCA::stopContent(bool allowBackoff, bool generateTxOp)
{
    EV_STATICCONTEXT

    //update all Queues

    EV << "Stopping Contention at " << omnetpp::simTime().raw() << std::endl;

    omnetpp::simtime_t passedTime = omnetpp::simTime() - lastStart;

    EV << "Channel was idle for " << passedTime << std::endl;

    lastStart = -1; //indicate that there was no last start

    for (auto &iter : myQueues)
    {
        if (iter.second.currentBackoff != 0 || iter.second.queue.size() != 0)
        {
            //check how many slots we already waited until the channel became busy

            int64_t oldBackoff = iter.second.currentBackoff;

            std::string info;
            if (passedTime < iter.second.aifsn * SLOTLENGTH_11P + SIFS_11P)
            {
                //we didn't even make it one DIFS :(
                info.append(" No DIFS");
            }
            else
            {
                //decrease the backoff by one because we made it longer than one DIFS
                iter.second.currentBackoff--;

                //check how many slots we waited after the first DIFS
                int64_t passedSlots = (int64_t)((passedTime - omnetpp::SimTime(iter.second.aifsn * SLOTLENGTH_11P + SIFS_11P)) / SLOTLENGTH_11P);

                EV << "Passed slots after DIFS: " << passedSlots << std::endl;

                if (iter.second.queue.size() == 0)
                {
                    //this can be below 0 because of post transmit backoff -> backoff on empty queues will not generate macevents,
                    //we dont want to generate a txOP for empty queues
                    iter.second.currentBackoff -= std::min(iter.second.currentBackoff,passedSlots);
                    info.append(" PostCommit Over");
                }
                else
                {
                    iter.second.currentBackoff -= passedSlots;
                    if (iter.second.currentBackoff <= -1)
                    {
                        if (generateTxOp)
                        {
                            iter.second.txOP = true; info.append(" TXOP");
                        }
                        //else: this packet couldn't be sent because there was too little time. we could have generated a txop, but the channel switched
                        iter.second.currentBackoff = 0;
                    }

                }
            }

            EV << "Updating backoff for Queue " << iter.first << ": " << oldBackoff << " -> " << iter.second.currentBackoff << info <<std::endl;
        }
    }
}


WaveShortMessage* EDCA::initiateTransmit(omnetpp::simtime_t lastIdle)
{
    EV_STATICCONTEXT

    //iterate through the queues to return the packet we want to send
    WaveShortMessage* pktToSend = NULL;

    omnetpp::simtime_t idleTime = omnetpp::simTime() - lastIdle;

    EV << "Initiating transmit at " << omnetpp::simTime() << ". I've been idle since " << idleTime << std::endl;

    // As t_access_category is sorted by priority, we iterate back to front.
    // This realizes the behavior documented in IEEE Std 802.11-2012 Section 9.2.4.2;
    // that is, "data frames from the higher priority AC" win an internal collision.
    // The phrase "EDCAF of higher UP" of IEEE Std 802.11-2012 Section 9.19.2.3 is assumed to be meaningless.
    for (auto iter = myQueues.rbegin(); iter != myQueues.rend(); iter++)
    {
        if (iter->second.queue.size() != 0)
        {
            if (idleTime >= (iter->second.aifsn * SLOTLENGTH_11P) + SIFS_11P && iter->second.txOP)
            {
                EV << "Queue " << iter->first << " is ready to send! \n";

                iter->second.txOP = false;

                // this queue is ready to send
                if (pktToSend == NULL)
                {
                    pktToSend = iter->second.queue.front();
                }
                else
                {
                    // there was already another packet ready.
                    // we have to go increase cw and go into backoff.
                    // It's called internal contention and its wonderful
                    NumInternalContention++;

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


void EDCA::backoff(t_access_category ac)
{
    EV_STATICCONTEXT

    myQueues[ac].currentBackoff = owner->intuniform(0,myQueues[ac].cwCur);
    SlotsBackoff += myQueues[ac].currentBackoff;
    NumBackoff++;

    EV << "Going into Backoff because channel was busy when new packet arrived from upperLayer" << std::endl;
}


void EDCA::postTransmit(t_access_category ac)
{
    EV_STATICCONTEXT

    // delete the MAC frame object
    delete myQueues[ac].queue.front();
    // remove it from the queue
    myQueues[ac].queue.pop();

    // reset current CW back to default
    myQueues[ac].cwCur = myQueues[ac].cwMin;
    // and set the post-transmit backoff
    myQueues[ac].currentBackoff = owner->intuniform(0, myQueues[ac].cwCur);

    // update the statistics
    SlotsBackoff += myQueues[ac].currentBackoff;
    NumBackoff++;

    EV << "Queue " << ac << " will go into post-transmit backoff for " << myQueues[ac].currentBackoff << " slots" << std::endl;
}


void EDCA::revokeTxOPs()
{
    for (auto &iter : myQueues)
    {
        if (iter.second.txOP)
        {
            iter.second.txOP = false;
            iter.second.currentBackoff = 0;
        }
    }
}


void EDCA::cleanUp()
{
    for (auto &iter : myQueues)
    {
        while (iter.second.queue.size() != 0)
        {
            delete iter.second.queue.front();
            iter.second.queue.pop();
        }
    }

    myQueues.clear();
}

}

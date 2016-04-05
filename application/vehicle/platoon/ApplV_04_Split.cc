/****************************************************************************/
/// @file    ApplV_04_Split.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    August 2013
///
/****************************************************************************/
// VENTOS, Vehicular Network Open Simulator; see http:?
// Copyright (C) 2013-2015
/****************************************************************************/
//
// This file is part of VENTOS.
// VENTOS is free software; you can redistribute it and/or modify
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

#include "ApplV_02_PlatoonMg.h"

namespace VENTOS {

void ApplVPlatoonMg::split_handleSelfMsg(cMessage* msg)
{
    if(!splitEnabled)
        return;

    if(msg == mgrTIMER)
    {
        splitMonitor();
    }
    else if(msg == plnTIMER4)
    {
        vehicleState = state_sendSplitReq;
        reportStateToStat();

        split_DataFSM();
    }
    else if(msg == plnTIMER6)
    {
        if(vehicleState == state_waitForAck)
        {
            vehicleState = state_makeItFreeAgent;
            reportStateToStat();

            split_DataFSM();
        }
    }
    else if(msg == plnTIMER7)
    {
        if(vehicleState == state_waitForAllAcks2)
        {
            TotalACKsRx = 0;
            TotalPLSent = 0;

            vehicleState = state_changePL;
            reportStateToStat();

            split_DataFSM();
        }
    }
    else if(msg == plnTIMER5)
    {
        if(vehicleState == state_waitForCHANGEPL)
        {
            vehicleState = state_platoonFollower;
            reportStateToStat();
        }
    }
    else if(msg == plnTIMER8)
    {
        if(vehicleState == state_waitForSplitDone)
        {
            vehicleState = state_sendingACK;
            reportStateToStat();

            split_DataFSM();
        }
    }
    else if(msg == plnTIMER8a)
    {
        if( GapCreated() )
        {
            vehicleState = state_platoonLeader;
            reportStateToStat();

            // MyCircularBufferSplit.clear();

            reportManeuverToStat(SUMOID, "-", "Split_End");

            // send a unicast GAP_CREATED to the old leader
            // we need this in follower/leader leave only. Other maneuvers ignore this
            PlatoonMsg* dataMsg = prepareData(oldPlnID, GAP_CREATED, oldPlnID);
            EV << "### " << SUMOID << ": sent GAP_CREATED." << endl;
            sendDelayed(dataMsg, individualOffset, lowerLayerOut);
            reportCommandToStat(dataMsg);
        }
        else
            scheduleAt(simTime() + 0.1, plnTIMER8a);
    }
}


// platoon leader checks constantly if we can split
void ApplVPlatoonMg::splitMonitor()
{
    if(vehicleState == state_platoonLeader)
    {
        if(!busy && splitEnabled && plnSize > optPlnSize)
        {
            splittingDepth = optPlnSize;
            splittingVehicle = plnMembersList[splittingDepth];
            splitCaller = -1;

            busy = true;

            vehicleState = state_sendSplitReq;
            reportStateToStat();

            split_DataFSM();
        }
    }

    scheduleAt(simTime() + 0.1, mgrTIMER);
}


void ApplVPlatoonMg::split_BeaconFSM(BeaconVehicle *wsm)
{
    if(!splitEnabled)
        return;
}


void ApplVPlatoonMg::split_DataFSM(PlatoonMsg *wsm)
{
    if(!splitEnabled)
        return;

    if(vehicleState == state_sendSplitReq)
    {
        if(plnSize == -1)
            error("WTH! platoon size is -1!");

        // check if splittingDepth is valid
        if(splittingDepth <= 0 || splittingDepth >= plnSize)
            error("splitting depth is wrong!");

        // check if splittingVehicle is valid
        if(splittingVehicle == "")
            error("splitting vehicle is not known!");

        // send a unicast SPLIT_REQ to the follower
        PlatoonMsg* dataMsg = prepareData(splittingVehicle, SPLIT_REQ, plnID);
        EV << "### " << SUMOID << ": sent SPLIT_REQ." << endl;
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        vehicleState = state_waitForSplitReply;
        reportStateToStat();

        scheduleAt(simTime() + 1., plnTIMER4);
    }
    else if(vehicleState == state_waitForSplitReply)
    {
        if(wsm->getType() == SPLIT_ACCEPT && wsm->getSender() == splittingVehicle)
        {
            cancelEvent(plnTIMER4);

            vehicleState = state_makeItFreeAgent;
            reportStateToStat();

            reportManeuverToStat(splittingVehicle, "-", "Split_Start");

            split_DataFSM();
        }
    }
    else if(vehicleState == state_makeItFreeAgent)
    {
        // send CHANGE_PL to the splitting vehicle (last two parameters are data attached to this ucommand)
        PlatoonMsg* dataMsg = prepareData(splittingVehicle, CHANGE_PL, plnID, (-splittingDepth), splittingVehicle);
        EV << "### " << SUMOID << ": sent CHANGE_PL." << endl;
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        vehicleState = state_waitForAck;
        reportStateToStat();

        scheduleAt(simTime() + 5., plnTIMER6);
    }
    else if(vehicleState == state_waitForAck)
    {
        if (wsm->getType() == ACK && wsm->getSender() == splittingVehicle)
        {
            cancelEvent(plnTIMER6);

            // store the elements of second platoon before erasing
            secondPlnMembersList.clear();
            for (std::deque<std::string>::iterator it=plnMembersList.begin()+splittingDepth; it!=plnMembersList.end(); ++it)
                secondPlnMembersList.push_back(*it);

            // if splitting vehicle is the last follower
            // i.e: no follower exists after the splitting vehicle
            if(splittingDepth + 1 == plnSize)
            {
                vehicleState = state_splitDone;
                reportStateToStat();
            }
            else
            {
                vehicleState = state_changePL;
                reportStateToStat();
            }

            split_DataFSM();
        }
    }
    else if(vehicleState == state_splitDone)
    {
        plnSize = splittingDepth;
        plnMembersList.pop_back();

        updateColorDepth();

        if( adaptiveTG && plnSize < (maxPlnSize / 2) )
        {
            // decrease Tg
            PlatoonMsg* dataMsg = prepareData("multicast", CHANGE_Tg, plnID, TG1);
            EV << "### " << SUMOID << ": sent CHANGE_Tg with value " << TG1 << endl;
            sendDelayed(dataMsg, individualOffset, lowerLayerOut);
            reportCommandToStat(dataMsg);
        }

        // send unicast SPLIT_DONE
        // we send two data to our follower:
        // 1) splitCaller 2) our platoon member list
        PlatoonMsg* dataMsg = prepareData(splittingVehicle, SPLIT_DONE, plnID, splitCaller, "", secondPlnMembersList);
        EV << "### " << SUMOID << ": sent SPLIT_DONE." << endl;
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        if(splitCaller == -1)
        {
            busy = false;

            vehicleState = state_platoonLeader;
            reportStateToStat();
        }
        // leader leave had called split
        // we should not set busy to false; leader leave is on-going
        else if(splitCaller == 0)
        {
            vehicleState = state_splitCompleted;
            reportStateToStat();
        }
        // follower leave had called split
        // we should not set busy to false; follower leave is on-going
        else if(splitCaller == 1)
        {
            vehicleState = state_platoonLeader;
            reportStateToStat();
        }
    }
    else if(vehicleState == state_changePL)
    {
        // send CHANGE_PL to followers after the splitting vehicle
        for (std::deque<std::string>::iterator it=plnMembersList.begin()+splittingDepth+1; it!=plnMembersList.end(); ++it)
        {
            std::string targetVeh = *it;
            PlatoonMsg* dataMsg = prepareData(targetVeh, CHANGE_PL, plnID, (-splittingDepth), splittingVehicle);
            EV << "### " << SUMOID << ": sent CHANGE_PL." << endl;
            simtime_t offset = dblrand() * maxOffset;
            sendDelayed(dataMsg, offset, lowerLayerOut);
            reportCommandToStat(dataMsg);

            TotalPLSent++;
        }

        vehicleState = state_waitForAllAcks2;
        reportStateToStat();

        scheduleAt(simTime() + 1., plnTIMER7);
    }
    else if(vehicleState == state_waitForAllAcks2)
    {
        if (wsm->getType() == ACK && wsm->getReceivingPlatoonID() == plnID)
        {
            std::string followerID = wsm->getSender();
            RemoveFollowerFromList_Split(followerID);

            TotalACKsRx++;

            // all followers ACK-ed
            if(TotalACKsRx == TotalPLSent)
            {
                cancelEvent(plnTIMER7);
                TotalACKsRx = 0;
                TotalPLSent = 0;

                vehicleState = state_splitDone;
                reportStateToStat();

                split_DataFSM();
            }
        }
    }
    else if(vehicleState == state_platoonFollower)
    {
        // splitting vehicle receives a SPLIT_REQ
        if (wsm->getType() == SPLIT_REQ && wsm->getRecipient() == SUMOID)
        {
            // send SPLIT_ACCEPT
            PlatoonMsg* dataMsg = prepareData(plnID, SPLIT_ACCEPT, plnID);
            EV << "### " << SUMOID << ": sent SPLIT_ACCEPT." << endl;
            sendDelayed(dataMsg, individualOffset, lowerLayerOut);
            reportCommandToStat(dataMsg);

            vehicleState = state_waitForCHANGEPL;
            reportStateToStat();

            scheduleAt(simTime() + 1., plnTIMER5);
        }
    }
    else if(vehicleState == state_waitForCHANGEPL)
    {
        if (wsm->getType() == CHANGE_PL && wsm->getSender() == plnID && wsm->getRecipient() == SUMOID)
        {
            cancelEvent(plnTIMER5);

            // save my old platoon leader info for future use
            oldPlnID = plnID;

            // I am a free agent now!
            plnID = wsm->getStrValue();
            myPlnDepth = myPlnDepth + wsm->getDblValue();
            plnSize = 1;

            // change color to red!
            RGB newColor = Color::colorNameToRGB("red");
            TraCI->vehicleSetColor(SUMOID, newColor);

            vehicleState = state_sendingACK;
            reportStateToStat();

            split_DataFSM();
        }
    }
    else if(vehicleState == state_sendingACK)
    {
        // send ACK
        PlatoonMsg* dataMsg = prepareData(oldPlnID, ACK, plnID);
        EV << "### " << SUMOID << ": sent ACK." << endl;
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        vehicleState = state_waitForSplitDone;
        reportStateToStat();

        scheduleAt(simTime() + 1., plnTIMER8);
    }
    else if(vehicleState == state_waitForSplitDone)
    {
        if(wsm->getType() == SPLIT_DONE && wsm->getSender() == oldPlnID && wsm->getRecipient() == SUMOID)
        {
            cancelEvent(plnTIMER8);

            plnMembersList.clear();
            plnMembersList = wsm->getQueueValue();

            plnSize = plnMembersList.size();

            updateColorDepth();

            TraCI->vehicleSetTimeGap(SUMOID, TP);

            // check splitCaller. If 'leader leave' is on-going
            if(wsm->getDblValue() == 0)
            {
                // then check if there is any leading vehicle after my leader
                std::vector<std::string> vleaderIDnew = TraCI->vehicleGetLeader(oldPlnID, sonarDist);
                std::string vleaderID = vleaderIDnew[0];

                if(vleaderID == "")
                {
                    TraCI->vehicleSetSpeed(SUMOID, 20.);
                }
                // if yes
                else
                {
                    TraCI->vehicleSetSpeed(SUMOID, 30.); // set max speed
                }
            }
            else
            {
                TraCI->vehicleSetSpeed(SUMOID, 30.); // set max speed
            }

            vehicleState = state_waitForGap;
            reportStateToStat();

            // MyCircularBufferSplit.clear();

            // check each 0.1s to see if the gap is big enough
            scheduleAt(simTime() + .1, plnTIMER8a);
        }
    }
}


void ApplVPlatoonMg::RemoveFollowerFromList_Split(std::string followerID)
{
    bool found = false;
    unsigned int i = 0;

    for(i = 0; i < plnMembersList.size(); i++)
    {
        if(plnMembersList[i] == followerID)
        {
            found = true;
            break;
        }
    }

    if(found)
    {
        plnMembersList.erase(plnMembersList.begin() + i);
        plnSize--;
    }
}


bool ApplVPlatoonMg::GapCreated()
{
    // we use our sonar to check the gap
    std::vector<std::string> vleaderIDnew = TraCI->vehicleGetLeader(SUMOID, sonarDist);
    std::string vleaderID = vleaderIDnew[0];
    double gap = atof( vleaderIDnew[1].c_str() );

    if(vleaderID == "")
        return true;

    // get the timeGap setting
    double timeGapSetting = TraCI->vehicleGetTimeGap(SUMOID);

    // get speed
    double speed = TraCI->vehicleGetSpeed(SUMOID);

    // get minGap
    double minGap = TraCI->vehicleGetMinGap(SUMOID);

    double targetGap = (speed * timeGapSetting) + minGap;

    //cout << simTime().dbl() << ": " << SUMOID << ", speed = " << speed << ", targetGap = " << targetGap << ", gap = " << gap << endl;
    //if( gap >= targetGap ) cout << " BOOOM! " << endl;

    if( gap >= targetGap )
        return true;
    else
        return false;
}

}

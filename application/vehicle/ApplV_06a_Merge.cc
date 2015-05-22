/****************************************************************************/
/// @file    ApplV_06a_Merge.cc
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

#include "ApplV_06_PlatoonMg.h"

namespace VENTOS {

void ApplVPlatoonMg::merge_handleSelfMsg(cMessage* msg)
{
    if(!mergeEnabled)
        return;

    if(msg == plnTIMER1)
    {
        if(vehicleState == state_waitForMergeReply)
        {
            // leader does not response after three re-attempts!
            if(mergeReqAttempts >= 3)
            {
                mergeReqAttempts = 0;

                vehicleState = state_platoonLeader;
                reportStateToStat();
            }
            else
            {
                vehicleState = state_sendMergeReq;
                reportStateToStat();

                merge_BeaconFSM();
            }
        }
    }
    else if(msg == plnTIMER1a)
    {
        if(vehicleState == state_waitForCatchup)
        {
            // check gap to the last follower
            if( CatchUpDone() )
            {
                // MyCircularBufferMerge.clear();

                // free agent
                if(plnSize == 1)
                {
                    vehicleState = state_sendMergeDone;
                    reportStateToStat();

                    merge_DataFSM();
                }
                else
                {
                    vehicleState = state_notifyFollowers;
                    reportStateToStat();

                    merge_DataFSM();
                }
            }
            else
                scheduleAt(simTime() + 0.1, plnTIMER1a);
        }
    }
    else if(msg == plnTIMER2)
    {
        if(vehicleState == state_waitForAllAcks)
        {
            vehicleState = state_notifyFollowers;
            reportStateToStat();

            merge_DataFSM();
        }
    }
    else if(msg == plnTIMER3)
    {
        if(vehicleState == state_waitForMergeDone)
        {
            vehicleState = state_sendMergeAccept;
            reportStateToStat();

            merge_DataFSM();
        }
    }
}


void ApplVPlatoonMg::merge_BeaconFSM(BeaconVehicle* wsm)
{
    if(!mergeEnabled)
        return;

    if(vehicleState == state_platoonLeader)
    {
        // can we merge?
        if(!busy && plnSize < optPlnSize)
        {
            if(isBeaconFromLeading(wsm))
            {
                int finalPlnSize = wsm->getPlatoonDepth() + 1 + plnSize;

                if(finalPlnSize <= optPlnSize)
                {
                    vehicleState = state_sendMergeReq;
                    reportStateToStat();

                    merge_BeaconFSM(wsm);
                }
            }
        }
    }
    else if(vehicleState == state_sendMergeReq)
    {
        // save these values from the received beacon from my preceding vehicle
        if(wsm != NULL)
        {
            leadingPlnID = wsm->getPlatoonID();
            leadingPlnDepth = wsm->getPlatoonDepth();
        }

        // if its not a valid leader id or depth!
        if(leadingPlnID == "" || leadingPlnDepth == -1)
        {
            vehicleState = state_platoonLeader;
            reportStateToStat();

            return;
        }

        // send a unicast MERGE_REQ to its platoon leader
        PlatoonMsg* dataMsg = prepareData(leadingPlnID, MERGE_REQ, leadingPlnID, -1, "", plnMembersList);
        EV << "### " << SUMOvID << ": sent MERGE_REQ." << endl;
        printDataContent(dataMsg);
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        mergeReqAttempts++;

        vehicleState = state_waitForMergeReply;
        reportStateToStat();

        reportManeuverToStat(SUMOvID, leadingPlnID, "Merge_Request");

        // start plnTIMER1
        scheduleAt(simTime() + 1., plnTIMER1);
    }
}


void ApplVPlatoonMg::merge_DataFSM(PlatoonMsg* wsm)
{
    if(!mergeEnabled)
        return;

    if(vehicleState == state_waitForMergeReply)
    {
        if (wsm->getType() == MERGE_REJECT && wsm->getSender() == leadingPlnID)
        {
            mergeReqAttempts = 0;

            cancelEvent(plnTIMER1);

            vehicleState = state_platoonLeader;
            reportStateToStat();

            reportManeuverToStat(SUMOvID, "-", "Merge_Reject");
        }
        else if(wsm->getType() == MERGE_ACCEPT && wsm->getSender() == leadingPlnID)
        {
            mergeReqAttempts = 0;

            vehicleState = state_mergeAccepted;
            reportStateToStat();

            reportManeuverToStat(SUMOvID, "-", "Merge_Start");

            merge_DataFSM();
        }
    }
    else if(vehicleState == state_mergeAccepted)
    {
        cancelEvent(plnTIMER1);
        TraCI->vehicleSetTimeGap(SUMOvID, TG1);
        TraCI->vehicleSetSpeed(SUMOvID, 30.);  // catch-up

        // now we should wait until we catch-up completely
        vehicleState = state_waitForCatchup;
        reportStateToStat();

        // MyCircularBufferMerge.clear();

        scheduleAt(simTime() + .1, plnTIMER1a);
    }
    else if(vehicleState == state_waitForCatchup)
    {
        // if we are in waitForCatchup and receive a
        // MERGE_REQ then we should reject it!
        if (wsm->getType() == MERGE_REQ && wsm->getRecipient() == plnID)
        {
            // send MERGE_REJECT
            PlatoonMsg* dataMsg = prepareData(wsm->getSender(), MERGE_REJECT, wsm->getSendingPlatoonID());
            EV << "### " << SUMOvID << ": sent MERGE_REJECT." << endl;
            printDataContent(dataMsg);
            sendDelayed(dataMsg, individualOffset, lowerLayerOut);
            reportCommandToStat(dataMsg);
        }
    }
    else if(vehicleState == state_sendMergeDone)
    {
        plnID = leadingPlnID;
        myPlnDepth = leadingPlnDepth + 1;
        plnSize = -1;
        plnMembersList.clear();

        // send unicast MERGE_DONE
        PlatoonMsg* dataMsg = prepareData(plnID, MERGE_DONE, plnID);
        EV << "### " << SUMOvID << ": sent MERGE_DONE." << endl;
        printDataContent(dataMsg);
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        vehicleState = state_platoonFollower;
        reportStateToStat();

        reportManeuverToStat(SUMOvID, "-", "Merge_End");
    }
    else if(vehicleState == state_notifyFollowers)
    {
        // send CHANGE_PL to all my followers (last two parameters are data attached to this ucommand)
        PlatoonMsg* dataMsg = prepareData("multicast", CHANGE_PL, leadingPlnID, leadingPlnDepth+1, leadingPlnID);
        EV << "### " << SUMOvID << ": sent CHANGE_PL." << endl;
        printDataContent(dataMsg);
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        vehicleState = state_waitForAllAcks;
        reportStateToStat();

        // start plnTIMER2
        scheduleAt(simTime() + 1., plnTIMER2);
    }
    else if(vehicleState == state_waitForAllAcks)
    {
        if ( wsm->getType() == ACK && (wsm->getSendingPlatoonID() == plnID || wsm->getSendingPlatoonID() == leadingPlnID) )
        {
            std::string followerID = wsm->getSender();
            RemoveFollowerFromList_Merge(followerID);

            // all followers ACK-ed
            if(plnSize == 1)
            {
                cancelEvent(plnTIMER2);
                vehicleState = state_sendMergeDone;
                reportStateToStat();

                merge_DataFSM();
            }
        }
    }
    else if(vehicleState == state_platoonLeader)
    {
        if (wsm->getType() == MERGE_REQ && wsm->getRecipient() == plnID)
        {
            int finalPlnSize =  wsm->getQueueValue().size() + plnSize;

            if(busy || finalPlnSize > optPlnSize)
            {
                // send MERGE_REJECT
                PlatoonMsg* dataMsg = prepareData(wsm->getSender(), MERGE_REJECT, wsm->getSendingPlatoonID());
                EV << "### " << SUMOvID << ": sent MERGE_REJECT." << endl;
                printDataContent(dataMsg);
                sendDelayed(dataMsg, individualOffset, lowerLayerOut);
                reportCommandToStat(dataMsg);
            }
            else
            {
                // save followers list for future use
                secondPlnMembersList = wsm->getQueueValue();

                vehicleState = state_sendMergeAccept;
                reportStateToStat();

                merge_DataFSM(wsm);
            }
        }
    }
    else if(vehicleState == state_sendMergeAccept)
    {
        // send MERGE_ACCEPT
        PlatoonMsg* dataMsg = prepareData(secondPlnMembersList.front().c_str(), MERGE_ACCEPT, secondPlnMembersList.front().c_str());
        EV << "### " << SUMOvID << ": sent MERGE_ACCEPT." << endl;
        printDataContent(dataMsg);
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        // we start the merge maneuver
        busy = true;

        vehicleState = state_waitForMergeDone;
        reportStateToStat();

        // start plnTIMER3 (we wait for 5 seconds!)
        scheduleAt(simTime() + 5., plnTIMER3);
    }
    else if(vehicleState == state_waitForMergeDone)
    {
        if(wsm->getType() == MERGE_DONE && wsm->getRecipient() == plnID)
        {
            vehicleState = state_mergeDone;
            reportStateToStat();

            merge_DataFSM();
        }
    }
    else if(vehicleState == state_mergeDone)
    {
        cancelEvent(plnTIMER3);
        plnSize = plnSize + secondPlnMembersList.size();
        plnMembersList.insert(plnMembersList.end(), secondPlnMembersList.begin(), secondPlnMembersList.end());

        if( adaptiveTG && plnSize > (maxPlnSize / 2) )
        {
            // increase Tg
            PlatoonMsg* dataMsg = prepareData("multicast", CHANGE_Tg, plnID, TG2);
            EV << "### " << SUMOvID << ": sent CHANGE_Tg with value " << TG2 << endl;
            printDataContent(dataMsg);
            sendDelayed(dataMsg, individualOffset, lowerLayerOut);
            reportCommandToStat(dataMsg);
        }

        vehicleState = state_platoonLeader;
        reportStateToStat();

        // now that plnSize is changed, we should
        // change the color of the followers
        updateColorDepth();

        busy = false;
    }
}


void ApplVPlatoonMg::RemoveFollowerFromList_Merge(std::string followerID)
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


bool ApplVPlatoonMg::CatchUpDone()
{
    // we use our sonar to check the gap
    std::vector<std::string> vleaderIDnew = TraCI->vehicleGetLeader(SUMOvID, sonarDist);
    std::string vleaderID = vleaderIDnew[0];
    double gap = atof( vleaderIDnew[1].c_str() );

    if(vleaderID == "")
        return true;

    // get the timeGap setting
    double timeGapSetting = TraCI->vehicleGetTimeGap(SUMOvID);

    // get speed
    double speed = TraCI->vehicleGetSpeed(SUMOvID);

    // get minGap
    double minGap = TraCI->vehicleGetMinGap(SUMOvID);

    double targetGap = (speed * timeGapSetting) + minGap;

    if( (0.95 * gap) <= targetGap )
        return true;
    else
        return false;

    /*
    // store the current gap
    MyCircularBufferMerge.push_back(gap);

    // we should wait for the buffer to be filled completely
    if(MyCircularBufferMerge.size() < MAX_BUFF_MERGE)
        return false;

    // calculate sum
    double sum = 0;
    for (boost::circular_buffer<double>::iterator it = MyCircularBufferMerge.begin(); it != MyCircularBufferMerge.end(); it++)
        sum = sum + *it;

    // calculate average
    double avg = sum / MyCircularBufferMerge.size();

    // calculate variance
    double var = 0;
    for (boost::circular_buffer<double>::iterator it = MyCircularBufferMerge.begin(); it != MyCircularBufferMerge.end(); it++)
       var = var + pow(fabs(*it - avg), 2);

    if(var < 0.1)
    {
        //cout << SUMOvID << ": merge done!" << endl;
        return true;
    }
    else
        return false;
    */
}

}



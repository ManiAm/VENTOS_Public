
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
                scheduleAt(simTime() + 0.5, plnTIMER1a);
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
        mergeReqAttempts = 0;

        if (wsm->getType() == MERGE_REJECT && wsm->getSender() == leadingPlnID)
        {
            cancelEvent(plnTIMER1);

            vehicleState = state_platoonLeader;
            reportStateToStat();

            reportManeuverToStat(SUMOvID, "-", "Merge_Reject");
        }
        else if(wsm->getType() == MERGE_ACCEPT && wsm->getSender() == leadingPlnID)
        {
            vehicleState = state_mergeAccepted;
            reportStateToStat();

            reportManeuverToStat(SUMOvID, "-", "Merge_Start");

            merge_DataFSM();
        }
    }
    else if(vehicleState == state_mergeAccepted)
    {
        cancelEvent(plnTIMER1);
        TraCI->commandSetTg(SUMOvID, 0.55);
        TraCI->commandSetSpeed(SUMOvID, 30.);  // catch-up

        // now we should wait until we catch-up completely
        vehicleState = state_waitForCatchup;
        reportStateToStat();

        scheduleAt(simTime() + 1., plnTIMER1a);
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
        if (wsm->getType() == ACK && wsm->getSendingPlatoonID() == plnID)
        {
            string followerID = wsm->getSender();
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

        if(plnSize > (maxPlnSize / 2))
        {
            // increase Tg
            PlatoonMsg* dataMsg = prepareData("multicast", CHANGE_Tg, plnID, 0.6);
            EV << "### " << SUMOvID << ": sent CHANGE_Tg with value " << 0.6 << endl;
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


void ApplVPlatoonMg::RemoveFollowerFromList_Merge(string followerID)
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
    vector<string> vleaderIDnew = TraCI->commandGetLeading(SUMOvID, sonarDist);
    string vleaderID = vleaderIDnew[0];
    double gap = atof( vleaderIDnew[1].c_str() );

//    double timeGap = 0.55; // TraCI->commandGetVehicleMinGap(SUMOvID);
//    double speed = TraCI->commandGetVehicleSpeed(SUMOvID);
//    double minGap = TraCI->commandGetVehicleMinGap(SUMOvID);
//
//    double catchupGap = (timeGap * speed) + minGap;

    if(vleaderID == "" || gap < 10)
        return true;

    return false;
}
}




#include "ApplV_05_PlatoonMg.h"


void ApplVPlatoonMg::merge_handleSelfMsg(cMessage* msg)
{
    if(msg == plnTIMER1)
    {
        if(vehicleState == state_waitForMergeReply)
        {
            vehicleState = state_sendMergeReq;

            // report to statistics
            CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
            simsignal_t Signal_VehicleState = registerSignal("VehicleState");
            nodePtr->emit(Signal_VehicleState, state);

            merge_BeaconFSM();
        }
    }
    else if(msg == plnTIMER1a)
    {
        if(vehicleState == state_waitForCatchup)
        {
            // todo:
            // check gap to the last follower
            if(true)
            {
                // free agent
                if(plnSize == 1)
                {
                    vehicleState = state_sendMergeDone;

                    // report to statistics
                    CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
                    simsignal_t Signal_VehicleState = registerSignal("VehicleState");
                    nodePtr->emit(Signal_VehicleState, state);

                    merge_DataFSM();
                }
                else
                {
                    vehicleState = state_notifyFollowers;

                    // report to statistics
                    CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
                    simsignal_t Signal_VehicleState = registerSignal("VehicleState");
                    nodePtr->emit(Signal_VehicleState, state);

                    merge_DataFSM();
                }
            }
            else
                scheduleAt(simTime() + 1., plnTIMER1a);
        }
    }
    else if(msg == plnTIMER2)
    {
        if(vehicleState == state_waitForAllAcks)
        {
            vehicleState = state_notifyFollowers;

            // report to statistics
            CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
            simsignal_t Signal_VehicleState = registerSignal("VehicleState");
            nodePtr->emit(Signal_VehicleState, state);

            merge_DataFSM();
        }
    }
    else if(msg == plnTIMER3)
    {
        if(vehicleState == state_waitForMergeDone)
        {
            vehicleState = state_sendMergeAccept;

            // report to statistics
            CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
            simsignal_t Signal_VehicleState = registerSignal("VehicleState");
            nodePtr->emit(Signal_VehicleState, state);

            merge_DataFSM();
        }
    }
}


void ApplVPlatoonMg::merge_BeaconFSM(BeaconVehicle* wsm)
{
    if(vehicleState == state_platoonLeader)
    {
        // Can we merge?
        if(plnSize < optPlnSize && isBeaconFromLeading(wsm))
        {
            vehicleState = state_sendMergeReq;

            // report to statistics
            CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
            simsignal_t Signal_VehicleState = registerSignal("VehicleState");
            nodePtr->emit(Signal_VehicleState, state);

            merge_BeaconFSM(wsm);
        }
    }
    else if(vehicleState == state_sendMergeReq)
    {
        // save these values from beacon of leading vehicle
        if(wsm != NULL)
        {
            leadingPlnID = wsm->getPlatoonID();
            leadingPlnDepth = wsm->getPlatoonDepth();
        }

        // if its not a valid leader id!
        if(leadingPlnID == "")
        {
            vehicleState = state_platoonLeader;

            // report to statistics
            CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
            simsignal_t Signal_VehicleState = registerSignal("VehicleState");
            nodePtr->emit(Signal_VehicleState, state);

            return;
        }

        // send a unicast MERGE_REQ to its platoon leader
        PlatoonMsg* dataMsg = prepareData(leadingPlnID, MERGE_REQ, leadingPlnID, -1, "", plnMembersList);
        EV << "### " << SUMOvID << ": sent MERGE_REQ." << endl;
        printDataContent(dataMsg);
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);

        // report to statistics
        CurrentPlnMsg *plnMsg = new CurrentPlnMsg(dataMsg->getSender(), dataMsg->getRecipient(), uCommandToStr(dataMsg->getType()).c_str(), dataMsg->getSendingPlatoonID(), dataMsg->getReceivingPlatoonID());
        simsignal_t Signal_SentPlatoonMsg = registerSignal("SentPlatoonMsg");
        nodePtr->emit(Signal_SentPlatoonMsg, plnMsg);

        vehicleState = state_waitForMergeReply;

        // report to statistics
        CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
        simsignal_t Signal_VehicleState = registerSignal("VehicleState");
        nodePtr->emit(Signal_VehicleState, state);

        // start plnTIMER1
        scheduleAt(simTime() + 1., plnTIMER1);
    }
}


void ApplVPlatoonMg::merge_DataFSM(PlatoonMsg* wsm)
{
    if(vehicleState == state_waitForMergeReply)
    {
        if (wsm->getType() == MERGE_REJECT && wsm->getSender() == leadingPlnID)
        {
            cancelEvent(plnTIMER1);
            vehicleState = state_platoonLeader;

            // report to statistics
            CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
            simsignal_t Signal_VehicleState = registerSignal("VehicleState");
            nodePtr->emit(Signal_VehicleState, state);
        }
        else if(wsm->getType() == MERGE_ACCEPT && wsm->getSender() == leadingPlnID)
        {
            vehicleState = state_mergeAccepted;

            // report to statistics
            CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
            simsignal_t Signal_VehicleState = registerSignal("VehicleState");
            nodePtr->emit(Signal_VehicleState, state);

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

        // report to statistics
        CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
        simsignal_t Signal_VehicleState = registerSignal("VehicleState");
        nodePtr->emit(Signal_VehicleState, state);

        scheduleAt(simTime() + 1., plnTIMER1a);
    }
    else if(vehicleState == state_waitForCatchup)
    {
        if (wsm->getType() == MERGE_REQ && wsm->getRecipient() == plnID)
        {
            // send MERGE_REJECT
            PlatoonMsg* dataMsg = prepareData(wsm->getSender(), MERGE_REJECT, wsm->getSendingPlatoonID());
            EV << "### " << SUMOvID << ": sent MERGE_REJECT." << endl;
            printDataContent(dataMsg);
            sendDelayed(dataMsg, individualOffset, lowerLayerOut);

            // report to statistics
            string str = uCommandToStr(dataMsg->getType()) + " (busy)";
            CurrentPlnMsg *plnMsg = new CurrentPlnMsg(dataMsg->getSender(), dataMsg->getRecipient(), str.c_str(), dataMsg->getSendingPlatoonID(), dataMsg->getReceivingPlatoonID());
            simsignal_t Signal_SentPlatoonMsg = registerSignal("SentPlatoonMsg");
            nodePtr->emit(Signal_SentPlatoonMsg, plnMsg);
        }
    }
    else if(vehicleState == state_sendMergeDone)
    {
        plnID = leadingPlnID;
        myPlnDepth = leadingPlnDepth + 1;
        plnSize = -1;
        plnMembersList.clear();

        // update follower color
        updateColor();

        // send unicast MERGE_DONE
        PlatoonMsg* dataMsg = prepareData(plnID, MERGE_DONE, plnID);
        EV << "### " << SUMOvID << ": sent MERGE_DONE." << endl;
        printDataContent(dataMsg);
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);

        // report to statistics
        CurrentPlnMsg *plnMsg = new CurrentPlnMsg(dataMsg->getSender(), dataMsg->getRecipient(), uCommandToStr(dataMsg->getType()).c_str(), dataMsg->getSendingPlatoonID(), dataMsg->getReceivingPlatoonID());
        simsignal_t Signal_SentPlatoonMsg = registerSignal("SentPlatoonMsg");
        nodePtr->emit(Signal_SentPlatoonMsg, plnMsg);

        vehicleState = state_platoonMember;

        // report to statistics
        CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
        simsignal_t Signal_VehicleState = registerSignal("VehicleState");
        nodePtr->emit(Signal_VehicleState, state);
    }
    else if(vehicleState == state_notifyFollowers)
    {
        // send CHANGE_PL to all my followers (last two parameters are data attached to this ucommand)
        PlatoonMsg* dataMsg = prepareData("multicast", CHANGE_PL, leadingPlnID, leadingPlnDepth+1, leadingPlnID);
        EV << "### " << SUMOvID << ": sent CHANGE_PL." << endl;
        printDataContent(dataMsg);
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);

        // report to statistics
        CurrentPlnMsg *plnMsg = new CurrentPlnMsg(dataMsg->getSender(), dataMsg->getRecipient(), uCommandToStr(dataMsg->getType()).c_str(), dataMsg->getSendingPlatoonID(), dataMsg->getReceivingPlatoonID());
        simsignal_t Signal_SentPlatoonMsg = registerSignal("SentPlatoonMsg");
        nodePtr->emit(Signal_SentPlatoonMsg, plnMsg);

        vehicleState = state_waitForAllAcks;

        // report to statistics
        CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
        simsignal_t Signal_VehicleState = registerSignal("VehicleState");
        nodePtr->emit(Signal_VehicleState, state);

        // start plnTIMER2
        scheduleAt(simTime() + 1., plnTIMER2);
    }
    else if(vehicleState == state_waitForAllAcks)
    {
        if (wsm->getType() == ACK && wsm->getSendingPlatoonID() == plnID)
        {
            string followerID = wsm->getSender();
            RemoveFollowerFromList(followerID);

            // all followers ACK-ed
            if(plnSize == 1)
            {
                cancelEvent(plnTIMER2);
                vehicleState = state_sendMergeDone;

                // report to statistics
                CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
                simsignal_t Signal_VehicleState = registerSignal("VehicleState");
                nodePtr->emit(Signal_VehicleState, state);

                merge_DataFSM();
            }
        }
    }
    else if(vehicleState == state_platoonMember)
    {
        if (wsm->getType() == CHANGE_PL && wsm->getSender() == plnID)
        {
            plnID = wsm->getStrValue();
            myPlnDepth = myPlnDepth + wsm->getDblValue();
            updateColor();

            // send ACK
            PlatoonMsg* dataMsg = prepareData(wsm->getSender(), ACK, wsm->getSendingPlatoonID());
            EV << "### " << SUMOvID << ": sent ACK." << endl;
            printDataContent(dataMsg);
            sendDelayed(dataMsg, individualOffset, lowerLayerOut);

            // report to statistics
            CurrentPlnMsg *plnMsg = new CurrentPlnMsg(dataMsg->getSender(), dataMsg->getRecipient(), uCommandToStr(dataMsg->getType()).c_str(), dataMsg->getSendingPlatoonID(), dataMsg->getReceivingPlatoonID());
            simsignal_t Signal_SentPlatoonMsg = registerSignal("SentPlatoonMsg");
            nodePtr->emit(Signal_SentPlatoonMsg, plnMsg);
        }
        else if(wsm->getType() == CHANGE_Tg && wsm->getSender() == plnID)
        {
            TraCI->commandSetTg(SUMOvID, wsm->getDblValue());
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

                // report to statistics
                string str;
                if(busy) str = uCommandToStr(dataMsg->getType()) + " (busy)";
                else str = uCommandToStr(dataMsg->getType()) + " (optPlnSize)";
                CurrentPlnMsg *plnMsg = new CurrentPlnMsg(dataMsg->getSender(), dataMsg->getRecipient(), str.c_str(), dataMsg->getSendingPlatoonID(), dataMsg->getReceivingPlatoonID());
                simsignal_t Signal_SentPlatoonMsg = registerSignal("SentPlatoonMsg");
                nodePtr->emit(Signal_SentPlatoonMsg, plnMsg);
            }
            else
            {
                // save followers list for future use
                secondPlnMembersList = wsm->getQueueValue();

                vehicleState = state_sendMergeAccept;

                // report to statistics
                CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
                simsignal_t Signal_VehicleState = registerSignal("VehicleState");
                nodePtr->emit(Signal_VehicleState, state);

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

        // report to statistics
        CurrentPlnMsg *plnMsg = new CurrentPlnMsg(dataMsg->getSender(), dataMsg->getRecipient(), uCommandToStr(dataMsg->getType()).c_str(), dataMsg->getSendingPlatoonID(), dataMsg->getReceivingPlatoonID());
        simsignal_t Signal_SentPlatoonMsg = registerSignal("SentPlatoonMsg");
        nodePtr->emit(Signal_SentPlatoonMsg, plnMsg);

        // we start the merge maneuver
        busy = true;

        vehicleState = state_waitForMergeDone;

        // report to statistics
        CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
        simsignal_t Signal_VehicleState = registerSignal("VehicleState");
        nodePtr->emit(Signal_VehicleState, state);

        // start plnTIMER3
        scheduleAt(simTime() + 1., plnTIMER3);
    }
    else if(vehicleState == state_waitForMergeDone)
    {
        if(wsm->getType() == MERGE_DONE && wsm->getRecipient() == plnID)
        {
            vehicleState = state_mergeDone;

            // report to statistics
            CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
            simsignal_t Signal_VehicleState = registerSignal("VehicleState");
            nodePtr->emit(Signal_VehicleState, state);

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

            // report to statistics
            CurrentPlnMsg *plnMsg = new CurrentPlnMsg(dataMsg->getSender(), dataMsg->getRecipient(), uCommandToStr(dataMsg->getType()).c_str(), dataMsg->getSendingPlatoonID(), dataMsg->getReceivingPlatoonID());
            simsignal_t Signal_SentPlatoonMsg = registerSignal("SentPlatoonMsg");
            nodePtr->emit(Signal_SentPlatoonMsg, plnMsg);
        }

        busy = false;

        vehicleState = state_platoonLeader;

        // report to statistics
        CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
        simsignal_t Signal_VehicleState = registerSignal("VehicleState");
        nodePtr->emit(Signal_VehicleState, state);
    }
}


void ApplVPlatoonMg::RemoveFollowerFromList(string followerID)
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



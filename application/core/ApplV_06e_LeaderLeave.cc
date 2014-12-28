
#include "ApplV_06_PlatoonMg.h"

namespace VENTOS {

void ApplVPlatoonMg::leaderLeave_handleSelfMsg(cMessage* msg)
{
    if(!leaderLeaveEnabled)
        return;

    if(msg == plnTIMER9)
    {
        if(vehicleState == state_waitForVoteReply)
        {
            vehicleState = state_sendVoteLeader;
            reportStateToStat();

            leaderLeave_DataFSM();
        }
    }
}


void ApplVPlatoonMg::leaderLeave_BeaconFSM(BeaconVehicle *wsm)
{
    if(!leaderLeaveEnabled)
        return;
}


void ApplVPlatoonMg::leaderLeave_DataFSM(PlatoonMsg *wsm)
{
    if(!leaderLeaveEnabled)
        return;

    if(vehicleState == state_sendVoteLeader)
    {
        // send a multicast VOTE_LEADER to all followers
        PlatoonMsg* dataMsg = prepareData("multicast", VOTE_LEADER, plnID);
        EV << "### " << SUMOvID << ": sent VOTE_LEADER." << endl;
        printDataContent(dataMsg);
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        vehicleState = state_waitForVoteReply;
        reportStateToStat();

        reportManeuverToStat(SUMOvID, "-", "LLeave_Start");

        scheduleAt(simTime() + 1., plnTIMER9);
    }
    else if(vehicleState == state_waitForVoteReply)
    {
        if(wsm->getType() == ELECTED_LEADER && wsm->getRecipient() == SUMOvID)
        {
            cancelEvent(plnTIMER9);

            // todo: it is always 1 for now!
            splittingDepth = 1;
            splittingVehicle = plnMembersList[splittingDepth];
            splitCaller = 0;  // Notifying split that leader leave is the caller

            vehicleState = state_sendSplitReq;
            reportStateToStat();

            split_DataFSM();
        }
    }
    else if(vehicleState == state_splitCompleted)
    {
        // now we can leave the platoon
        if(wsm->getType() == GAP_CREATED && wsm->getRecipient() == SUMOvID)
        {
            TraCI->commandSetvClass(SUMOvID, "private");   // change vClass

            int32_t bitset = TraCI->commandMakeLaneChangeMode(10, 01, 01, 01, 01);
            TraCI->commandSetLaneChangeMode(SUMOvID, bitset);  // alter 'lane change' mode
            TraCI->commandChangeLane(SUMOvID, 0, 5);   // change to lane 0 (normal lane)

            TraCI->commandSetSpeed(SUMOvID, 30.);

            // change color to yellow!
            TraCIColor newColor = TraCIColor::fromTkColor("yellow");
            TraCI->commandSetVehicleColor(SUMOvID, newColor);

            plnSize = -1;
            myPlnDepth = -1;
            busy = false;

            reportManeuverToStat(SUMOvID, "-", "LLeave_End");

            vehicleState = state_idle;
            reportStateToStat();
        }
    }
    else if(vehicleState == state_platoonFollower)
    {
        if ( wsm->getType() == VOTE_LEADER && wsm->getSender() == plnID )
        {
            // todo:
            // we assume the second vehicle in the platoon always replies
            if(myPlnDepth == 1)
            {
                // send ELECTED_LEADER
                PlatoonMsg* dataMsg = prepareData(plnID, ELECTED_LEADER, plnID, myPlnDepth);
                EV << "### " << SUMOvID << ": sent ELECTED_LEADER." << endl;
                printDataContent(dataMsg);
                sendDelayed(dataMsg, individualOffset, lowerLayerOut);
                reportCommandToStat(dataMsg);
            }
        }
    }
}

}

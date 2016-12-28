/****************************************************************************/
/// @file    ApplV_07_LeaderLeave.cc
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

#include "vehicle/platoon/02_PlatoonMg.h"

namespace VENTOS {

void ApplVPlatoonMg::leaderLeave_handleSelfMsg(omnetpp::cMessage* msg)
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
        EV << "### " << SUMOID << ": sent VOTE_LEADER." << std::endl;
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        vehicleState = state_waitForVoteReply;
        reportStateToStat();

        reportManeuverToStat(SUMOID, "-", "LLeave_Start");

        scheduleAt(omnetpp::simTime() + 1., plnTIMER9);
    }
    else if(vehicleState == state_waitForVoteReply)
    {
        if(wsm->getType() == ELECTED_LEADER && wsm->getRecipient() == SUMOID)
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
        if(wsm->getType() == GAP_CREATED && wsm->getRecipient() == SUMOID)
        {
            TraCI->vehicleSetClass(SUMOID, "private");   // change vClass

            int32_t bitset = TraCI->vehicleBuildLaneChangeMode(10, 01, 01, 01, 01);
            TraCI->vehicleSetLaneChangeMode(SUMOID, bitset);  // alter 'lane change' mode
            TraCI->vehicleChangeLane(SUMOID, 0, 5);   // change to lane 0 (normal lane)

            TraCI->vehicleSetSpeed(SUMOID, 30.);

            // change color to yellow!
            RGB newColor = Color::colorNameToRGB("yellow");
            TraCI->vehicleSetColor(SUMOID, newColor);

            plnSize = -1;
            myPlnDepth = -1;
            busy = false;

            reportManeuverToStat(SUMOID, "-", "LLeave_End");

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
                EV << "### " << SUMOID << ": sent ELECTED_LEADER." << std::endl;
                sendDelayed(dataMsg, individualOffset, lowerLayerOut);
                reportCommandToStat(dataMsg);
            }
        }
    }
}

}

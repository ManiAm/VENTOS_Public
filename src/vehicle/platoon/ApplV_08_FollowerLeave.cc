/****************************************************************************/
/// @file    ApplV_08_FollowerLeave.cc
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

void ApplVPlatoonMg::followerLeave_handleSelfMsg(omnetpp::cMessage* msg)
{
    if(!followerLeaveEnabled)
        return;

    if(msg == plnTIMER10)
    {
        if(vehicleState == state_waitForLeaveReply)
        {
            vehicleState = state_sendLeaveReq;
            reportStateToStat();

            followerLeave_DataFSM();
        }
    }
    else if(msg == plnTIMER11)
    {
        // check if we are free agent?
        if(vehicleState == state_platoonLeader && plnSize == 1)
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

            reportManeuverToStat(SUMOID, "-", "FLeave_End");

            vehicleState = state_idle;
            reportStateToStat();
        }
        else
            scheduleAt(omnetpp::simTime() + .1, plnTIMER11);
    }
}


void ApplVPlatoonMg::followerLeave_BeaconFSM(BeaconVehicle *wsm)
{
    if(!followerLeaveEnabled)
        return;

}


void ApplVPlatoonMg::followerLeave_DataFSM(PlatoonMsg *wsm)
{
    if(!followerLeaveEnabled)
        return;

    if(vehicleState == state_sendLeaveReq)
    {
        // send a unicast LEAVE_REQ to the leader
        PlatoonMsg* dataMsg = prepareData(plnID, LEAVE_REQ, plnID, myPlnDepth);
        EV << "### " << SUMOID << ": sent LEAVE_REQ." << std::endl;
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        vehicleState = state_waitForLeaveReply;
        reportStateToStat();

        reportManeuverToStat(SUMOID, plnID, "FLeave_Request");

        scheduleAt(omnetpp::simTime() + 5., plnTIMER10);
    }
    else if(vehicleState == state_waitForLeaveReply)
    {
        if(wsm->getType() == LEAVE_REJECT && wsm->getSender() == plnID)
        {
            cancelEvent(plnTIMER10);

            reportManeuverToStat(SUMOID, "-", "FLeave_Reject");

            vehicleState = state_platoonFollower;
            reportStateToStat();
        }
        else if(wsm->getType() == LEAVE_ACCEPT && wsm->getSender() == plnID)
        {
            cancelEvent(plnTIMER10);

            if(wsm->getDblValue() == 0)
                reportManeuverToStat(SUMOID, "-", "MFLeave_Start");
            else if(wsm->getDblValue() == 1)
                reportManeuverToStat(SUMOID, "-", "LFLeave_Start");
            else
                error("Unknwon value!");

            vehicleState = state_platoonFollower;
            reportStateToStat();

            // now we should wait for the leader to do the split(s), and make us a free agent.
            // we check every 0.1s to see if we are free agent
            scheduleAt(omnetpp::simTime() + .1, plnTIMER11);
        }
    }
    else if(vehicleState == state_platoonLeader)
    {
        // leader receives a LEAVE_REQ
        if (wsm->getType() == LEAVE_REQ && wsm->getRecipient() == SUMOID)
        {
            if(wsm->getDblValue() <= 0 || wsm->getDblValue() >= plnSize)
                error("depth of the follower is not right!");

            busy = true;

            int lastFollower = (wsm->getDblValue() + 1 == plnSize) ? 1 : 0;

            // send LEAVE_ACCEPT
            // lastFollower notifies the leaving vehicle if it is the last follower or not!
            PlatoonMsg* dataMsg = prepareData(wsm->getSender(), LEAVE_ACCEPT, plnID, lastFollower);
            EV << "### " << SUMOID << ": sent LEAVE_ACCEPT." << std::endl;
            sendDelayed(dataMsg, individualOffset, lowerLayerOut);
            reportCommandToStat(dataMsg);

            // last follower wants to leave
            // one split is enough
            if(wsm->getDblValue() + 1 == plnSize)
            {
                RemainingSplits = 1;

                splittingDepth = wsm->getDblValue();
                splittingVehicle = plnMembersList[splittingDepth];
                splitCaller = 1;  // Notifying split that follower leave is the caller

                vehicleState = state_sendSplitReq;
                reportStateToStat();

                split_DataFSM();
            }
            // middle follower wants to leave
            // we need two splits
            else
            {
                RemainingSplits = 2;

                // start the first split
                splittingDepth = wsm->getDblValue() + 1;
                splittingVehicle = plnMembersList[splittingDepth];
                splitCaller = 1;  // Notifying split that follower leave is the caller

                vehicleState = state_sendSplitReq;
                reportStateToStat();

                split_DataFSM();
            }
        }
        // leader receives a GAP_CREATED
        else if(wsm->getType() == GAP_CREATED && wsm->getRecipient() == SUMOID)
        {
            RemainingSplits--;

            if(RemainingSplits == 0)
            {
                // no more splits are needed. We are done!
                busy = false;
            }
            else if(RemainingSplits == 1)
            {
                // start the second split
                splittingDepth = plnSize - 1;
                splittingVehicle = plnMembersList[splittingDepth];
                splitCaller = 1;  // Notifying split that follower leave is the caller

                vehicleState = state_sendSplitReq;
                reportStateToStat();

                split_DataFSM();
            }
        }
    }
}

}

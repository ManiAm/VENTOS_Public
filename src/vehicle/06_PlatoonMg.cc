/****************************************************************************/
/// @file    ApplV_02_PlatoonMg.cc
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

#include "vehicle/06_PlatoonMg.h"
#include "global/SignalObj.h"

namespace VENTOS {

Define_Module(VENTOS::ApplVPlatoonMg);

ApplVPlatoonMg::~ApplVPlatoonMg()
{

}


void ApplVPlatoonMg::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        if(plnMode != platoonManagement)
            return;

        if(!DSRCenabled)
            throw omnetpp::cRuntimeError("This vehicle is not DSRC-enabled!");

        maxPlnSize = par("maxPlatoonSize").longValue();
        optPlnSize = par("optPlatoonSize").longValue();

        adaptiveTG = par("adaptiveTG").boolValue();
        TP = par("TP").doubleValue();    // 3.5 s
        TG1 = par("TG1").doubleValue();  // 0.55 s
        TG2 = par("TG2").doubleValue();  // 0.6 s

        entryEnabled = par("entryEnabled").boolValue();
        mergeEnabled = par("mergeEnabled").boolValue();
        splitEnabled = par("splitEnabled").boolValue();
        followerLeaveEnabled = par("followerLeaveEnabled").boolValue();
        leaderLeaveEnabled = par("leaderLeaveEnabled").boolValue();

        WATCH(vehicleState);
        WATCH(busy);

        // entry maneuver
        // --------------
        if(entryEnabled)
        {
            entryManeuverEvt = new omnetpp::cMessage("EntryEvt", TYPE_TIMER);
            //double offset = dblrand() * 10;
            scheduleAt(omnetpp::simTime() + 4., entryManeuverEvt); // todo: no offset for now!

            plnTIMER0 = new omnetpp::cMessage("listening to beacons", TYPE_TIMER);
        }

        // merge maneuver
        // --------------
        plnTIMER1  = new omnetpp::cMessage("wait for merge reply", TYPE_TIMER);
        plnTIMER1a = new omnetpp::cMessage("wait to catchup", TYPE_TIMER);
        plnTIMER2  = new omnetpp::cMessage("wait for followers ack", TYPE_TIMER);
        plnTIMER3  = new omnetpp::cMessage("wait for merge done", TYPE_TIMER);

        // split maneuver
        // --------------
        plnTIMER4  = new omnetpp::cMessage("wait for split reply", TYPE_TIMER);
        plnTIMER6  = new omnetpp::cMessage("wait for free agent ACK", TYPE_TIMER);
        plnTIMER7  = new omnetpp::cMessage("wait for all ACKs", TYPE_TIMER);
        plnTIMER5  = new omnetpp::cMessage("wait for change_pl", TYPE_TIMER);
        plnTIMER8  = new omnetpp::cMessage("wait for split done", TYPE_TIMER);
        plnTIMER8a = new omnetpp::cMessage("wait for enough gap", TYPE_TIMER);

        mgrTIMER = new omnetpp::cMessage("manager", TYPE_TIMER);
        scheduleAt(omnetpp::simTime() + 0.1, mgrTIMER);

        // leader leave
        // ------------
        plnTIMER9 = new omnetpp::cMessage("wait for VOTE reply", TYPE_TIMER);

        // follower leave
        // --------------
        plnTIMER10 = new omnetpp::cMessage("wait for leave reply", TYPE_TIMER);
        plnTIMER11 = new omnetpp::cMessage("wait for split completion", TYPE_TIMER);

        // dissolve
        // --------
        plnTIMER12 = new omnetpp::cMessage("wait for DISSOLVE ACK", TYPE_TIMER);
    }
}


void ApplVPlatoonMg::finish()
{
    super::finish();
}


// is called, every time the position of vehicle changes
void ApplVPlatoonMg::handlePositionUpdate(cObject* obj)
{
    // pass it down!
    super::handlePositionUpdate(obj);
}


void ApplVPlatoonMg::handleSelfMsg(omnetpp::cMessage* msg)
{
    if(plnMode == platoonManagement && (msg == entryManeuverEvt || msg == plnTIMER0))
        entry_handleSelfMsg(msg);
    else if(plnMode == platoonManagement && (msg == plnTIMER1 || msg == plnTIMER1a || msg == plnTIMER2 || msg == plnTIMER3))
        merge_handleSelfMsg(msg);
    else if(plnMode == platoonManagement && (msg == mgrTIMER || msg == plnTIMER4 || msg == plnTIMER6 || msg == plnTIMER7 || msg == plnTIMER5 || msg == plnTIMER8 || msg == plnTIMER8a))
        split_handleSelfMsg(msg);
    else if(plnMode == platoonManagement && msg == plnTIMER9)
        leaderLeave_handleSelfMsg(msg);
    else if(plnMode == platoonManagement && (msg == plnTIMER10 || msg == plnTIMER11))
        followerLeave_handleSelfMsg(msg);
    else if(plnMode == platoonManagement && msg == plnTIMER12)
        dissolve_handleSelfMsg(msg);
    else
        super::handleSelfMsg(msg);
}


void ApplVPlatoonMg::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down!
    super::onBeaconVehicle(wsm);

    if(plnMode != platoonManagement)
        return;

    merge_BeaconFSM(wsm);
    split_BeaconFSM(wsm);
    common_BeaconFSM(wsm);
    entry_BeaconFSM(wsm);
    leaderLeave_BeaconFSM(wsm);
    followerLeave_BeaconFSM(wsm);
    dissolve_BeaconFSM(wsm);
}


void ApplVPlatoonMg::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down!
    super::onBeaconRSU(wsm);

    if(plnMode != platoonManagement)
        return;
}


void ApplVPlatoonMg::onPlatoonMsg(PlatoonMsg* wsm)
{
    // pass it down!
    super::onPlatoonMsg(wsm);

    if(plnMode != platoonManagement)
        return;

    merge_DataFSM(wsm);
    split_DataFSM(wsm);
    common_DataFSM(wsm);
    entry_DataFSM(wsm);
    leaderLeave_DataFSM(wsm);
    followerLeave_DataFSM(wsm);
    dissolve_DataFSM(wsm);
}


PlatoonMsg*  ApplVPlatoonMg::prepareData(std::string receiver, uCommand_t type, std::string receivingPlatoonID, double dblValue, std::string strValue, std::deque<std::string> vecValue)
{
    if(plnMode != platoonManagement)
        throw omnetpp::cRuntimeError("This application mode does not support platoon management!");

    PlatoonMsg* wsm = new PlatoonMsg("platoonMsg", TYPE_PLATOON_DATA);

    // add header length
    wsm->addBitLength(headerLength);

    // add payload length
    wsm->addBitLength(dataLengthBits);

    wsm->setWsmVersion(1);
    wsm->setSecurityType(1);

    if(dataOnSch)
        wsm->setChannelNumber(Veins::Channels::SCH1);
    else
        wsm->setChannelNumber(Veins::Channels::CCH);

    wsm->setDataRate(1);
    wsm->setPriority(dataPriority);
    wsm->setPsid(0);

    wsm->setSender(SUMOID.c_str());
    wsm->setRecipient(receiver.c_str());
    wsm->setType(type);
    wsm->setSendingPlatoonID(plnID.c_str());
    wsm->setReceivingPlatoonID(receivingPlatoonID.c_str());
    wsm->setDblValue(dblValue);
    wsm->setStrValue(strValue.c_str());
    wsm->setQueueValue(vecValue);

    return wsm;
}


// change the blue color of the follower to show depth
// only platoon leader can call this!
void ApplVPlatoonMg::updateColorDepth()
{
    if(plnSize <= 0)
        throw omnetpp::cRuntimeError("plnSize is not right!");

    if(plnSize == 1)
        return;

    RGB colorRGB = Color::colorNameToRGB("blue");
    HSV colorHSV = Color::rgb2hsv(colorRGB.red, colorRGB.green, colorRGB.blue);

    // get different saturation
    std::vector<double> shades = Color::generateColorShades(plnSize - 1);

    // leader has all the followers in plnMembersList list
    for(unsigned int depth = 1; depth < plnMembersList.size(); ++depth)
    {
        HSV newColorHSV = {colorHSV.hue, shades[depth-1], colorHSV.value};
        RGB newColorRGB = Color::hsv2rgb(newColorHSV.hue, newColorHSV.saturation, newColorHSV.value);
        TraCI->vehicleSetColor(plnMembersList[depth], newColorRGB);
    }
}


void ApplVPlatoonMg::reportStateToStat()
{
    if(record_platoon_stat)
    {
        plnManagement_t tmp = {omnetpp::simTime().dbl(), SUMOID, "-", stateToStr(vehicleState), "-", "-"};
        STAT->global_plnManagement_stat.push_back(tmp);
    }
}


const std::string ApplVPlatoonMg::stateToStr(int s)
{
    const char * statesStrings[] = {
            "state_idle", "state_platoonLeader", "state_platoonFollower",

            "state_waitForLaneChange",

            "state_sendMergeReq", "state_waitForMergeReply", "state_mergeAccepted", "state_waitForCatchup",
            "state_sendMergeDone", "state_notifyFollowers",
            "state_state_waitForAllAcks", "state_sendMergeAccept",
            "state_waitForMergeDone", "state_mergeDone",

            "state_sendSplitReq", "state_waitForSplitReply", "state_makeItFreeAgent",
            "state_waitForAck", "state_splitDone", "state_changePL", "state_waitForAllAcks2",
            "state_waitForCHANGEPL", "state_sendingACK", "state_waitForSplitDone", "state_waitForGap",

            "state_sendVoteLeader", "state_waitForVoteReply", "state_splitCompleted",

            "state_sendLeaveReq", "state_waitForLeaveReply", "state_secondSplit",

            "state_sendDissolve", "state_waitForDissolveAck",
    };

    return statesStrings[s];
}


void ApplVPlatoonMg::reportCommandToStat(PlatoonMsg* dataMsg)
{
    if(record_platoon_stat)
    {
        plnManagement_t tmp = {omnetpp::simTime().dbl(), dataMsg->getSender(), dataMsg->getRecipient(), uCommandToStr(dataMsg->getType()), dataMsg->getSendingPlatoonID(), dataMsg->getReceivingPlatoonID()};
        STAT->global_plnManagement_stat.push_back(tmp);
    }
}


const std::string ApplVPlatoonMg::uCommandToStr(int c)
{
    const char * uCommandStrings[] = {
            "MERGE_REQ", "MERGE_ACCEPT", "MERGE_REJECT", "MERGE_DONE",
            "SPLIT_REQ", "SPLIT_ACCEPT", "SPLIT_REJECT", "SPLIT_DONE",
            "CHANGE_PL", "CHANGE_Tg",
            "VOTE_LEADER", "ELECTED_LEADER", "DISSOLVE",
            "LEAVE_REQ", "LEAVE_ACCEPT", "LEAVE_REJECT", "GAP_CREATED",
            "ACK",
    };

    return uCommandStrings[c];
}


void ApplVPlatoonMg::reportManeuverToStat(std::string from, std::string to, std::string maneuver)
{
    if(record_platoon_stat)
    {
        plnStat_t tmp = {omnetpp::simTime().dbl(), from, to, maneuver};
        STAT->global_plnData_stat.push_back(tmp);
    }
}


// ask the platoon leader to manually initiate split at position 'depth'.
// only platoon leader can call this method!
void ApplVPlatoonMg::splitFromPlatoon(int depth)
{
    if(!DSRCenabled)
        throw omnetpp::cRuntimeError("This vehicle is not VANET-enabled!");

    if(vehicleState != state_platoonLeader)
        throw omnetpp::cRuntimeError("only platoon leader can initiate split!");

    if(depth <= 0 || depth > plnSize-1)
        throw omnetpp::cRuntimeError("depth of splitting vehicle is invalid!");

    if(!busy && splitEnabled)
    {
        splittingDepth = depth;
        splittingVehicle = plnMembersList[splittingDepth];
        splitCaller = -1;

        busy = true;

        vehicleState = state_sendSplitReq;
        reportStateToStat();

        split_DataFSM();
    }
}


void ApplVPlatoonMg::leavePlatoon()
{
    if(!DSRCenabled)
        throw omnetpp::cRuntimeError("This vehicle is not VANET-enabled!");

    // if I am leader
    if(vehicleState == state_platoonLeader)
    {
        if(!busy && leaderLeaveEnabled && plnSize > 1)
        {
            busy = true;

            vehicleState = state_sendVoteLeader;
            reportStateToStat();

            leaderLeave_DataFSM();
        }
    }
    // if I am follower
    else if(vehicleState == state_platoonFollower)
    {
        if(!busy && followerLeaveEnabled)
        {
            busy = true;

            vehicleState = state_sendLeaveReq;
            reportStateToStat();

            followerLeave_DataFSM();
        }
    }
    else
        throw omnetpp::cRuntimeError("vehicle should be in leader or follower states!");
}


void ApplVPlatoonMg::dissolvePlatoon()
{
    if(!DSRCenabled)
        throw omnetpp::cRuntimeError("This vehicle is not VANET-enabled!");

    if(vehicleState != state_platoonLeader)
        throw omnetpp::cRuntimeError("only platoon leader can break-up the platoon!");

    if(plnSize <= 1 || busy)
        return;

    busy = true;

    vehicleState = state_sendDissolve;
    reportStateToStat();

    dissolve_DataFSM();
}

// -------------------------------------------------------------------------------------------
// --------------------------------------[ Common ]-------------------------------------------
//--------------------------------------------------------------------------------------------

void ApplVPlatoonMg::common_handleSelfMsg(omnetpp::cMessage* msg)
{

}


void ApplVPlatoonMg::common_BeaconFSM(BeaconVehicle* wsm)
{

}


void ApplVPlatoonMg::common_DataFSM(PlatoonMsg* wsm)
{
    if(vehicleState == state_platoonFollower)
    {
        if ( wsm->getType() == CHANGE_PL && wsm->getSender() == plnID )
        {
            if( std::string(wsm->getRecipient()) == "multicast" || wsm->getRecipient() == SUMOID )
            {
                // send ACK
                PlatoonMsg* dataMsg = prepareData(wsm->getSender(), ACK, wsm->getSendingPlatoonID());
                EV << "### " << SUMOID << ": sent ACK." << std::endl;
                sendDelayed(dataMsg, individualOffset, lowerLayerOut);
                reportCommandToStat(dataMsg);

                // save my current platoon ID
                oldPlnID = plnID;

                // these should be updated after sending ACK!
                plnID = wsm->getStrValue();
                myPlnDepth = myPlnDepth + wsm->getDblValue();
            }
        }
        // if my old platoon leader asks me to change my leader to the current one
        // I have done it before, so I only ACK
        else if( wsm->getType() == CHANGE_PL && wsm->getSender() == oldPlnID && wsm->getStrValue() == plnID )
        {
            if( std::string(wsm->getRecipient()) == "multicast" || wsm->getRecipient() == SUMOID )
            {
                // send ACK
                PlatoonMsg* dataMsg = prepareData(wsm->getSender(), ACK, wsm->getSendingPlatoonID());
                EV << "### " << SUMOID << ": sent ACK." << std::endl;
                sendDelayed(dataMsg, individualOffset, lowerLayerOut);
                reportCommandToStat(dataMsg);
            }
        }
        else if(wsm->getType() == CHANGE_Tg && wsm->getSender() == plnID)
        {
            TraCI->vehicleSetTimeGap(SUMOID, wsm->getDblValue());
        }
    }
}


// -------------------------------------------------------------------------------------------
// ---------------------------------[ Entry maneuver ]----------------------------------------
//--------------------------------------------------------------------------------------------

void ApplVPlatoonMg::entry_handleSelfMsg(omnetpp::cMessage* msg)
{
    if(!entryEnabled)
        return;

    // start the Entry maneuver
    if(msg == entryManeuverEvt && vehicleState == state_idle)
    {
        // check if we are at lane 0
        if(TraCI->vehicleGetLaneIndex(SUMOID) == 0)
        {
            TraCI->vehicleSetClass(SUMOID, "vip");   // change vClass to vip

            int32_t bitset = TraCI->vehicleBuildLaneChangeMode(10, 01, 01, 01, 01);
            TraCI->vehicleSetLaneChangeMode(SUMOID, bitset);   // alter 'lane change' mode
            TraCI->vehicleChangeLane(SUMOID, 1, 5);   // change to lane 1 (special lane)

            // change state to waitForLaneChange
            vehicleState = state_waitForLaneChange;
            reportStateToStat();

            scheduleAt(omnetpp::simTime() + 0.1, plnTIMER0);
        }
    }
    else if(msg == plnTIMER0 && vehicleState == state_waitForLaneChange)
    {
        // check if we change lane
        if(TraCI->vehicleGetLaneIndex(SUMOID) == 1)
        {
            // make it a free agent
            plnID = SUMOID;
            myPlnDepth = 0;
            plnSize = 1;
            plnMembersList.push_back(SUMOID);
            TraCI->vehicleSetTimeGap(SUMOID, TP);

            busy = false;

            // get my leading vehicle
            std::vector<std::string> vleaderIDnew = TraCI->vehicleGetLeader(SUMOID, sonarDist);
            std::string vleaderID = vleaderIDnew[0];

            // if no leading, set speed to 5 m/s
            if(vleaderID == "")
            {
                TraCI->vehicleSetSpeed(SUMOID, 5.);
            }
            // if I have leading, set speed to max
            else
            {
                TraCI->vehicleSetSpeed(SUMOID, 30.);
            }

            // change color to red!
            RGB newColor = Color::colorNameToRGB("red");
            TraCI->vehicleSetColor(SUMOID, newColor);

            // change state to platoon leader
            vehicleState = state_platoonLeader;
            reportStateToStat();
        }
        else
            scheduleAt(omnetpp::simTime() + 0.1, plnTIMER0);
    }
}


void ApplVPlatoonMg::entry_BeaconFSM(BeaconVehicle* wsm)
{

}


void ApplVPlatoonMg::entry_DataFSM(PlatoonMsg *wsm)
{

}


// -------------------------------------------------------------------------------------------
// ---------------------------------[ Merge maneuver ]----------------------------------------
//--------------------------------------------------------------------------------------------

void ApplVPlatoonMg::merge_handleSelfMsg(omnetpp::cMessage* msg)
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
                scheduleAt(omnetpp::simTime() + 0.1, plnTIMER1a);
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
        EV << "### " << SUMOID << ": sent MERGE_REQ." << std::endl;
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        mergeReqAttempts++;

        vehicleState = state_waitForMergeReply;
        reportStateToStat();

        reportManeuverToStat(SUMOID, leadingPlnID, "Merge_Request");

        // start plnTIMER1
        scheduleAt(omnetpp::simTime() + 1., plnTIMER1);
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

            reportManeuverToStat(SUMOID, "-", "Merge_Reject");
        }
        else if(wsm->getType() == MERGE_ACCEPT && wsm->getSender() == leadingPlnID)
        {
            mergeReqAttempts = 0;

            vehicleState = state_mergeAccepted;
            reportStateToStat();

            reportManeuverToStat(SUMOID, "-", "Merge_Start");

            merge_DataFSM();
        }
    }
    else if(vehicleState == state_mergeAccepted)
    {
        cancelEvent(plnTIMER1);
        TraCI->vehicleSetTimeGap(SUMOID, TG1);
        TraCI->vehicleSetSpeed(SUMOID, 30.);  // catch-up

        // now we should wait until we catch-up completely
        vehicleState = state_waitForCatchup;
        reportStateToStat();

        // MyCircularBufferMerge.clear();

        scheduleAt(omnetpp::simTime() + .1, plnTIMER1a);
    }
    else if(vehicleState == state_waitForCatchup)
    {
        // if we are in waitForCatchup and receive a
        // MERGE_REQ then we should reject it!
        if (wsm->getType() == MERGE_REQ && wsm->getRecipient() == plnID)
        {
            // send MERGE_REJECT
            PlatoonMsg* dataMsg = prepareData(wsm->getSender(), MERGE_REJECT, wsm->getSendingPlatoonID());
            EV << "### " << SUMOID << ": sent MERGE_REJECT." << std::endl;
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
        EV << "### " << SUMOID << ": sent MERGE_DONE." << std::endl;
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        vehicleState = state_platoonFollower;
        reportStateToStat();

        reportManeuverToStat(SUMOID, "-", "Merge_End");
    }
    else if(vehicleState == state_notifyFollowers)
    {
        // send CHANGE_PL to all my followers (last two parameters are data attached to this ucommand)
        PlatoonMsg* dataMsg = prepareData("multicast", CHANGE_PL, leadingPlnID, leadingPlnDepth+1, leadingPlnID);
        EV << "### " << SUMOID << ": sent CHANGE_PL." << std::endl;
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        vehicleState = state_waitForAllAcks;
        reportStateToStat();

        // start plnTIMER2
        scheduleAt(omnetpp::simTime() + 1., plnTIMER2);
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
                EV << "### " << SUMOID << ": sent MERGE_REJECT." << std::endl;
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
        EV << "### " << SUMOID << ": sent MERGE_ACCEPT." << std::endl;
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        // we start the merge maneuver
        busy = true;

        vehicleState = state_waitForMergeDone;
        reportStateToStat();

        // start plnTIMER3 (we wait for 5 seconds!)
        scheduleAt(omnetpp::simTime() + 5., plnTIMER3);
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
            EV << "### " << SUMOID << ": sent CHANGE_Tg with value " << TG2 << std::endl;
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

    if( (0.95 * gap) <= targetGap )
        return true;
    else
        return false;
}


// -------------------------------------------------------------------------------------------
// ---------------------------------[ Split maneuver ]----------------------------------------
//--------------------------------------------------------------------------------------------

void ApplVPlatoonMg::split_handleSelfMsg(omnetpp::cMessage* msg)
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
            EV << "### " << SUMOID << ": sent GAP_CREATED." << std::endl;
            sendDelayed(dataMsg, individualOffset, lowerLayerOut);
            reportCommandToStat(dataMsg);
        }
        else
            scheduleAt(omnetpp::simTime() + 0.1, plnTIMER8a);
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

    scheduleAt(omnetpp::simTime() + 0.1, mgrTIMER);
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
            throw omnetpp::cRuntimeError("WTH! platoon size is -1!");

        // check if splittingDepth is valid
        if(splittingDepth <= 0 || splittingDepth >= plnSize)
            throw omnetpp::cRuntimeError("splitting depth is wrong!");

        // check if splittingVehicle is valid
        if(splittingVehicle == "")
            throw omnetpp::cRuntimeError("splitting vehicle is not known!");

        // send a unicast SPLIT_REQ to the follower
        PlatoonMsg* dataMsg = prepareData(splittingVehicle, SPLIT_REQ, plnID);
        EV << "### " << SUMOID << ": sent SPLIT_REQ." << std::endl;
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        vehicleState = state_waitForSplitReply;
        reportStateToStat();

        scheduleAt(omnetpp::simTime() + 1., plnTIMER4);
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
        EV << "### " << SUMOID << ": sent CHANGE_PL." << std::endl;
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        vehicleState = state_waitForAck;
        reportStateToStat();

        scheduleAt(omnetpp::simTime() + 5., plnTIMER6);
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
            EV << "### " << SUMOID << ": sent CHANGE_Tg with value " << TG1 << std::endl;
            sendDelayed(dataMsg, individualOffset, lowerLayerOut);
            reportCommandToStat(dataMsg);
        }

        // send unicast SPLIT_DONE
        // we send two data to our follower:
        // 1) splitCaller 2) our platoon member list
        PlatoonMsg* dataMsg = prepareData(splittingVehicle, SPLIT_DONE, plnID, splitCaller, "", secondPlnMembersList);
        EV << "### " << SUMOID << ": sent SPLIT_DONE." << std::endl;
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
            EV << "### " << SUMOID << ": sent CHANGE_PL." << std::endl;
            omnetpp::simtime_t offset = dblrand() * maxOffset;
            sendDelayed(dataMsg, offset, lowerLayerOut);
            reportCommandToStat(dataMsg);

            TotalPLSent++;
        }

        vehicleState = state_waitForAllAcks2;
        reportStateToStat();

        scheduleAt(omnetpp::simTime() + 1., plnTIMER7);
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
            EV << "### " << SUMOID << ": sent SPLIT_ACCEPT." << std::endl;
            sendDelayed(dataMsg, individualOffset, lowerLayerOut);
            reportCommandToStat(dataMsg);

            vehicleState = state_waitForCHANGEPL;
            reportStateToStat();

            scheduleAt(omnetpp::simTime() + 1., plnTIMER5);
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
        EV << "### " << SUMOID << ": sent ACK." << std::endl;
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        vehicleState = state_waitForSplitDone;
        reportStateToStat();

        scheduleAt(omnetpp::simTime() + 1., plnTIMER8);
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
            scheduleAt(omnetpp::simTime() + .1, plnTIMER8a);
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


// -------------------------------------------------------------------------------------------
// --------------------------[ Leader Leave maneuver ]----------------------------------------
//--------------------------------------------------------------------------------------------

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


// -------------------------------------------------------------------------------------------
// ---------------------------[ Follower Leave maneuver ]-------------------------------------
//--------------------------------------------------------------------------------------------

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
                throw omnetpp::cRuntimeError("Unknwon value!");

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
                throw omnetpp::cRuntimeError("depth of the follower is not right!");

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

// -------------------------------------------------------------------------------------------
// -----------------------------------[ Dissolve ]--------------------------------------------
//--------------------------------------------------------------------------------------------

void ApplVPlatoonMg::dissolve_handleSelfMsg(omnetpp::cMessage* msg)
{
    if(msg == plnTIMER12)
    {
        if(vehicleState == state_waitForDissolveAck)
        {
            vehicleState = state_sendDissolve;
            reportStateToStat();

            dissolve_DataFSM();
        }
    }


}


void ApplVPlatoonMg::dissolve_BeaconFSM(BeaconVehicle* wsm)
{


}


void ApplVPlatoonMg::dissolve_DataFSM(PlatoonMsg* wsm)
{
    if(vehicleState == state_sendDissolve)
    {
        // get the last vehicle
        std::deque<std::string>::iterator it = plnMembersList.end() - 1;
        lastVeh = *it;

        // send a unicast DISSOLVE message my follower
        PlatoonMsg* dataMsg = prepareData(lastVeh, DISSOLVE, plnID);
        EV << "### " << SUMOID << ": sent DISSOLVE to followers." << std::endl;
        sendDelayed(dataMsg, individualOffset, lowerLayerOut);
        reportCommandToStat(dataMsg);

        vehicleState = state_waitForDissolveAck;
        reportStateToStat();

        scheduleAt(omnetpp::simTime() + 1., plnTIMER12);
    }
    else if(vehicleState == state_waitForDissolveAck)
    {
        if (wsm->getType() == ACK && wsm->getSender() == lastVeh)
        {
            cancelEvent(plnTIMER12);

            plnMembersList.pop_back();

            if(plnMembersList.size() == 1)
            {
                vehicleState = state_platoonLeader;
                reportStateToStat();
            }

            vehicleState = state_sendDissolve;
            reportStateToStat();

            dissolve_DataFSM();
        }
    }
    else if(vehicleState == state_platoonFollower)
    {
        if ( wsm->getType() == DISSOLVE && wsm->getSender() == plnID && wsm->getRecipient() == SUMOID )
        {
            // send ACK
            PlatoonMsg* dataMsg = prepareData(wsm->getSender(), ACK, wsm->getSendingPlatoonID());
            EV << "### " << SUMOID << ": sent ACK." << std::endl;
            sendDelayed(dataMsg, individualOffset, lowerLayerOut);
            reportCommandToStat(dataMsg);

            // make it a free agent
            plnID = SUMOID;
            myPlnDepth = 0;
            plnSize = 1;
            plnMembersList.push_back(SUMOID);
            TraCI->vehicleSetTimeGap(SUMOID, TP);

            busy = false;

            // change color to red!
            RGB newColor = Color::colorNameToRGB("red");
            TraCI->vehicleSetColor(SUMOID, newColor);

            vehicleState = state_platoonLeader;
            reportStateToStat();
        }
    }
}

}

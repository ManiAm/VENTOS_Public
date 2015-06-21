/****************************************************************************/
/// @file    ApplV_06_PlatoonMg.cc
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

Define_Module(VENTOS::ApplVPlatoonMg);

ApplVPlatoonMg::~ApplVPlatoonMg()
{

}


void ApplVPlatoonMg::initialize(int stage)
{
    ApplVPlatoonFormed::initialize(stage);

	if (stage == 0)
	{
        if(plnMode != 3)
            return;

	    if(!VANETenabled)
	        error("This vehicle is not VANET-enabled!");

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

        vehicleState = state_idle;
        busy = false;

        WATCH(vehicleState);
        WATCH(busy);

        // Create a circular buffer of doubles with capacity MAX_BUFF
        //MyCircularBufferMerge.set_capacity(MAX_BUFF_MERGE);
        //MyCircularBufferSplit.set_capacity(MAX_BUFF_SPLIT);

        // used in entry maneuver
        // ----------------------
        if(entryEnabled)
        {
            entryManeuverEvt = new cMessage("EntryEvt", KIND_TIMER);
            //double offset = dblrand() * 10;
            scheduleAt(simTime() + 4., entryManeuverEvt); // todo: no offset for now!

            plnTIMER0 = new cMessage("listening to beacons", KIND_TIMER);
        }

        // used in merge maneuver
        // ----------------------
        mergeReqAttempts = 0;
        leadingPlnID = "";
        leadingPlnDepth = -1;

        plnTIMER1  = new cMessage("wait for merge reply", KIND_TIMER);
        plnTIMER1a = new cMessage("wait to catchup", KIND_TIMER);
        plnTIMER2  = new cMessage("wait for followers ack", KIND_TIMER);
        plnTIMER3  = new cMessage("wait for merge done", KIND_TIMER);

        // used in split maneuver
        // ----------------------
        splittingVehicle = "";
        splittingDepth = -1;
        oldPlnID = "";
        TotalPLSent = 0;
        TotalACKsRx = 0;
        splitCaller = -1;

        plnTIMER4  = new cMessage("wait for split reply", KIND_TIMER);
        plnTIMER6  = new cMessage("wait for free agent ACK", KIND_TIMER);
        plnTIMER7  = new cMessage("wait for all ACKs", KIND_TIMER);
        plnTIMER5  = new cMessage("wait for change_pl", KIND_TIMER);
        plnTIMER8  = new cMessage("wait for split done", KIND_TIMER);
        plnTIMER8a = new cMessage("wait for enough gap", KIND_TIMER);

        mgrTIMER = new cMessage("manager", KIND_TIMER);
        scheduleAt(simTime() + 0.1, mgrTIMER);

        // used in leader leave
        // --------------------
        plnTIMER9 = new cMessage("wait for VOTE reply", KIND_TIMER);

        // used in follower leave
        // ----------------------
        RemainingSplits = 0;
        plnTIMER10 = new cMessage("wait for leave reply", KIND_TIMER);
        plnTIMER11 = new cMessage("wait for split completion", KIND_TIMER);

        // used in dissolve
        // ----------------
        plnTIMER12 = new cMessage("wait for DISSOLVE ACK", KIND_TIMER);
	}
}


void ApplVPlatoonMg::finish()
{
    ApplVPlatoonFormed::finish();
}


// is called, every time the position of vehicle changes
void ApplVPlatoonMg::handlePositionUpdate(cObject* obj)
{
    // pass it down!
    ApplVPlatoonFormed::handlePositionUpdate(obj);
}


void ApplVPlatoonMg::handleSelfMsg(cMessage* msg)
{
    // pass it down!
    ApplVPlatoonFormed::handleSelfMsg(msg);

    if(plnMode != 3)
        return;

    merge_handleSelfMsg(msg);
    split_handleSelfMsg(msg);
    common_handleSelfMsg(msg);
    entry_handleSelfMsg(msg);
    leaderLeave_handleSelfMsg(msg);
    followerLeave_handleSelfMsg(msg);
    dissolve_handleSelfMsg(msg);
}


void ApplVPlatoonMg::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down!
    ApplVPlatoonFormed::onBeaconVehicle(wsm);

    if(plnMode != 3)
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
    ApplVPlatoonFormed::onBeaconRSU(wsm);

    if(plnMode != 3)
        return;
}


void ApplVPlatoonMg::onData(PlatoonMsg* wsm)
{
    // pass it down!
    ApplVPlatoonFormed::onData(wsm);

    if(plnMode != 3)
        return;

    merge_DataFSM(wsm);
    split_DataFSM(wsm);
    common_DataFSM(wsm);
    entry_DataFSM(wsm);
    leaderLeave_DataFSM(wsm);
    followerLeave_DataFSM(wsm);
    dissolve_DataFSM(wsm);
}


PlatoonMsg*  ApplVPlatoonMg::prepareData(std::string receiver, uCommands type, std::string receivingPlatoonID, double dblValue, std::string strValue, std::deque<std::string> vecValue)
{
    if(plnMode != 3)
    {
        error("This application mode does not support platoon management!");
    }

    PlatoonMsg* wsm = new PlatoonMsg("platoonMsg");

    // add header length
    wsm->addBitLength(headerLength);

    // add payload length
    wsm->addBitLength(dataLengthBits);

    wsm->setWsmVersion(1);
    wsm->setSecurityType(1);

    if(dataOnSch)
    {
        wsm->setChannelNumber(Channels::SCH1);
    }
    else
    {
        wsm->setChannelNumber(Channels::CCH);
    }

    wsm->setDataRate(1);
    wsm->setPriority(dataPriority);
    wsm->setPsid(0);

    wsm->setSender(SUMOvID.c_str());
    wsm->setRecipient(receiver.c_str());
    wsm->setType(type);
    wsm->setSendingPlatoonID(plnID.c_str());
    wsm->setReceivingPlatoonID(receivingPlatoonID.c_str());
    wsm->setDblValue(dblValue);
    wsm->setStrValue(strValue.c_str());
    wsm->setQueueValue(vecValue);

    return wsm;
}


// change follower blue color to show depth
// only platoon leader can call this!
void ApplVPlatoonMg::updateColorDepth()
{
    if(plnSize <= 0)
        error("plnSize is not right!");

    if(plnSize == 1)
        return;

    int offset = 255 / (plnSize-1);
    int *pickColor = new int[plnSize];
    pickColor[0] = -1;
    int count = 0;
    for(int i = 1; i < plnSize; ++i)
    {
        pickColor[i] = count;
        count = count + offset;
    }

    // leader has all the followers in plnMembersList list
    for(unsigned int depth = 1; depth < plnMembersList.size(); ++depth)
    {
        TraCIColor newColor = TraCIColor(pickColor[depth], pickColor[depth], 255, 255);
        TraCI->vehicleSetColor(plnMembersList[depth], newColor);
    }
}


void ApplVPlatoonMg::reportStateToStat()
{
    CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
    simsignal_t Signal_VehicleState = registerSignal("VehicleState");
    nodePtr->emit(Signal_VehicleState, state);
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
    CurrentPlnMsg *plnMsg = new CurrentPlnMsg(dataMsg, uCommandToStr(dataMsg->getType()).c_str());
    simsignal_t Signal_SentPlatoonMsg = registerSignal("SentPlatoonMsg");
    nodePtr->emit(Signal_SentPlatoonMsg, plnMsg);
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
    PlnManeuver *com = new PlnManeuver(from.c_str(), to.c_str(), maneuver.c_str());
    simsignal_t Signal_PlnManeuver = registerSignal("PlnManeuver");
    nodePtr->emit(Signal_PlnManeuver, com);
}


// ask the platoon leader to manually initiate split at position 'depth'.
// only platoon leader can call this method!
void ApplVPlatoonMg::splitFromPlatoon(int depth)
{
    if(!VANETenabled)
        error("This vehicle is not VANET-enabled!");

    if(vehicleState != state_platoonLeader)
        error("only platoon leader can initiate split!");

    if(depth <= 0 || depth > plnSize-1)
        error("depth of splitting vehicle is invalid!");

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
    if(!VANETenabled)
        error("This vehicle is not VANET-enabled!");

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
        error("vehicle should be in leader or follower states!");
}


void ApplVPlatoonMg::dissolvePlatoon()
{
    if(!VANETenabled)
        error("This vehicle is not VANET-enabled!");

    if(vehicleState != state_platoonLeader)
        error("only platoon leader can break-up the platoon!");

    if(plnSize <= 1 || busy)
        return;

    busy = true;

    vehicleState = state_sendDissolve;
    reportStateToStat();

    dissolve_DataFSM();
}


}


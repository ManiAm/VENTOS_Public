/****************************************************************************/
/// @file    ApplV_06_PlatoonMg.h
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

#ifndef APPLVPLATOONMG_H
#define APPLVPLATOONMG_H

#include "nodes/vehicle/04_Platoon.h"

namespace VENTOS {

class ApplVPlatoonMg : public ApplVPlatoon
{
protected:
    // NED variables
    int maxPlnSize = -1;
    int optPlnSize = -1;

    double TP; // time-gap between two platoons
    double TG; // time-gap between vehicles in a platoon
    bool adaptiveTG;

    bool entryEnabled;
    bool mergeEnabled;
    bool splitEnabled;
    bool followerLeaveEnabled;
    bool leaderLeaveEnabled;

private:
    typedef ApplVPlatoon super;

    bool busy = false;

    // --[ entry ]--
    omnetpp::cMessage* entryManeuverEvt = NULL;
    omnetpp::cMessage* plnTIMER0 = NULL;

    // --[ merge ]--
    int mergeReqAttempts = 0;
    std::string leadingPlnID = "";
    int leadingPlnDepth = -1;
    std::deque<std::string> secondPlnMembersList;

    omnetpp::cMessage* plnTIMER1 = NULL;
    omnetpp::cMessage* plnTIMER1a = NULL;
    omnetpp::cMessage* plnTIMER2 = NULL;
    omnetpp::cMessage* plnTIMER3 = NULL;
    omnetpp::cMessage* plnTIMER3a = NULL;

    // --[ split ]--
    std::string splittingVehicle = "";
    int splittingDepth = -1;
    std::string oldPlnID = "";
    int TotalPLSent = 0;
    int TotalACKsRx = 0;
    int splitCaller = -1;
    bool manualSplit = false;

    omnetpp::cMessage* plnTIMER4 = NULL;
    omnetpp::cMessage* plnTIMER5 = NULL;
    omnetpp::cMessage* plnTIMER6 = NULL;
    omnetpp::cMessage* plnTIMER7 = NULL;
    omnetpp::cMessage* plnTIMER8 = NULL;
    omnetpp::cMessage* plnTIMER8a = NULL;

    // --[ leader leave ]--
    omnetpp::cMessage* plnTIMER9 = NULL;

    // --[ follower leave ]--
    int RemainingSplits = 0;

    omnetpp::cMessage* plnTIMER10 = NULL;
    omnetpp::cMessage* plnTIMER11 = NULL;

    std::string leaveDirection = "";

    // --[ dissolve ]--
    omnetpp::cMessage* plnTIMER12 = NULL;
    std::string lastVeh = "";

    typedef enum states_num
    {
        state_idle,             // 0

        state_platoonLeader,    // 1
        state_platoonFollower,

        // entry states
        state_waitForLaneChange, // 3

        // merge states
        state_sendMergeReq,     // 4
        state_waitForMergeReply,
        state_mergeAccepted,
        state_waitForCatchup,
        state_sendMergeDone,
        state_notifyFollowers,
        state_waitForAllAcks,
        state_sendMergeAccept,
        state_waitForMergeDone,
        state_mergeDone,

        // split states
        state_sendSplitReq,      // 14
        state_waitForSplitReply,
        state_makeItFreeAgent,
        state_waitForAck,
        state_splitDone,
        state_changePL,
        state_waitForAllAcks2,
        state_waitForCHANGEPL,
        state_sendingACK,
        state_waitForSplitDone,
        state_waitForGap,

        // leader leave
        state_sendVoteLeader,   // 25
        state_waitForVoteReply,
        state_splitCompleted,

        // follower leave
        state_sendLeaveReq,   // 28
        state_waitForLeaveReply,
        state_secondSplit,

        // dissolve
        state_sendDissolve,   // 31
        state_waitForDissolveAck,

        state_waitForBeacon,

        state_MAX,
    } states_num_t;

    states_num_t vehicleState = state_idle;

    const std::map<states_num_t, std::string> vehicleStateMap = {

            {state_idle, "state_idle"},

            {state_platoonLeader, "state_platoonLeader"},
            {state_platoonFollower, "state_platoonFollower"},

            // entry states
            {state_waitForLaneChange, "state_waitForLaneChange"},

            // merge states
            {state_sendMergeReq, "state_sendMergeReq"},
            {state_waitForMergeReply, "state_waitForMergeReply"},
            {state_mergeAccepted, "state_mergeAccepted"},
            {state_waitForCatchup, "state_waitForCatchup"},
            {state_sendMergeDone, "state_sendMergeDone"},
            {state_notifyFollowers, "state_notifyFollowers"},
            {state_waitForAllAcks, "state_waitForAllAcks"},
            {state_sendMergeAccept, "state_sendMergeAccept"},
            {state_waitForMergeDone, "state_waitForMergeDone"},
            {state_mergeDone, "state_mergeDone"},

            {state_sendSplitReq, "state_sendSplitReq"},
            {state_waitForSplitReply, "state_waitForSplitReply"},
            {state_makeItFreeAgent, "state_makeItFreeAgent"},
            {state_waitForAck, "state_waitForAck"},
            {state_splitDone, "state_splitDone"},
            {state_changePL, "state_changePL"},
            {state_waitForAllAcks2, "state_waitForAllAcks2"},
            {state_waitForCHANGEPL, "state_waitForCHANGEPL"},
            {state_sendingACK, "state_sendingACK"},
            {state_waitForSplitDone, "state_waitForSplitDone"},
            {state_waitForGap, "state_waitForGap"},

            // leader leave
            {state_sendVoteLeader, "state_sendVoteLeader"},
            {state_waitForVoteReply, "state_waitForVoteReply"},
            {state_splitCompleted, "state_splitCompleted"},

            // follower leave
            {state_sendLeaveReq, "state_sendLeaveReq"},
            {state_waitForLeaveReply, "state_waitForLeaveReply"},
            {state_secondSplit, "state_secondSplit"},

            // dissolve
            {state_sendDissolve, "state_sendDissolve"},
            {state_waitForDissolveAck, "state_waitForDissolveAck"},

            {state_waitForBeacon, "state_waitForBeacon"}
    };

    typedef enum uCommand
    {
        MERGE_REQ,
        MERGE_ACCEPT,
        MERGE_REJECT,
        MERGE_DONE,

        SPLIT_REQ,
        SPLIT_ACCEPT,
        SPLIT_REJECT,
        SPLIT_DONE,

        CHANGE_PL,
        CHANGE_Tg,

        VOTE_LEADER,
        ELECTED_LEADER,
        DISSOLVE,

        LEAVE_REQ,
        LEAVE_ACCEPT,
        LEAVE_REJECT,
        GAP_CREATED,

        ACK,
    } uCommand_t;

    const std::map<uCommand_t, std::string> uCommandMap = {

            {MERGE_REQ, "MERGE_REQ"},
            {MERGE_ACCEPT, "MERGE_ACCEPT"},
            {MERGE_REJECT, "MERGE_REJECT"},
            {MERGE_DONE, "MERGE_DONE"},

            {SPLIT_REQ, "SPLIT_REQ"},
            {SPLIT_ACCEPT, "SPLIT_ACCEPT"},
            {SPLIT_REJECT, "SPLIT_REJECT"},
            {SPLIT_DONE, "SPLIT_DONE"},

            {CHANGE_PL, "CHANGE_PL"},
            {CHANGE_Tg, "CHANGE_Tg"},

            {VOTE_LEADER, "VOTE_LEADER"},
            {ELECTED_LEADER, "ELECTED_LEADER"},
            {DISSOLVE, "DISSOLVE"},

            {LEAVE_REQ, "LEAVE_REQ"},
            {LEAVE_ACCEPT, "LEAVE_ACCEPT"},
            {LEAVE_REJECT, "LEAVE_REJECT"},
            {GAP_CREATED, "GAP_CREATED"},

            {ACK, "ACK"}
    };

public:
    ~ApplVPlatoonMg();
    virtual void initialize(int stage);
    virtual void finish();

    void manualMerge();
    void splitFromPlatoon(int);
    void leavePlatoon(std::string = "free");
    void dissolvePlatoon();

    int getOptSize() {return this->optPlnSize;};
    int getMaxSize() {return this->maxPlnSize;};
    bool isBusy() {return this->busy;};

    void setOptSize(int optSize) {this->optPlnSize = optSize;};

    bool getStatus_entry(void) {return this->entryEnabled;};
    bool getStatus_merge(void) {return this->mergeEnabled;};
    bool getStatus_split(void) {return this->splitEnabled;};
    bool getStatus_followerLeave(void) {return this->followerLeaveEnabled;};
    bool getStatus_leaderLeave(void) {return this->leaderLeaveEnabled;};

    void setStatus_entry(bool status) {this->entryEnabled = status;};
    void setStatus_merge(bool status) {this->mergeEnabled = status;};
    void setStatus_split(bool status) {this->splitEnabled = status;};
    void setStatus_followerLeave(bool status) {this->followerLeaveEnabled = status;};
    void setStatus_leaderLeave(bool status) {this->leaderLeaveEnabled = status;};

protected:
    virtual void handleSelfMsg(omnetpp::cMessage*);
    virtual void handlePositionUpdate(cObject*);

    virtual void onBeaconVehicle(BeaconVehicle*);
    virtual void onBeaconRSU(BeaconRSU*);
    virtual void onPlatoonMsg(PlatoonMsg* wsm);

private:
    void sendPltData(std::string, uCommand_t, std::string, value_t value = value_t());
    void updateColorDepth();
    void setVehicleState(states_num_t vehState, std::string maneuver = "");

    // reporting to statistics
    void reportConfigToStat();
    const std::string stateToStr(states_num_t);
    const std::string uCommandToStr(uCommand_t);

    // common operations in maneuvers
    void common_handleSelfMsg(omnetpp::cMessage* msg);
    void common_BeaconFSM(BeaconVehicle *wsm = NULL);
    void common_DataFSM(PlatoonMsg *wsm = NULL);

    // entry
    void entry_handleSelfMsg(omnetpp::cMessage* msg);
    void entry_BeaconFSM(BeaconVehicle *wsm);
    void entry_DataFSM(PlatoonMsg *wsm = NULL);

    // merge
    void merge_handleSelfMsg(omnetpp::cMessage* msg);
    void merge_BeaconFSM(BeaconVehicle *wsm = NULL);
    void merge_DataFSM(PlatoonMsg *wsm = NULL);
    void RemoveFollowerFromList_Merge(std::string);
    bool CatchUpDone();

    // split
    void pltSplitMonitor();
    void split_handleSelfMsg(omnetpp::cMessage* msg);
    void split_BeaconFSM(BeaconVehicle *wsm = NULL);
    void split_DataFSM(PlatoonMsg *wsm = NULL);
    void RemoveFollowerFromList_Split(std::string);
    bool GapCreated();

    // leader leave
    void leaderLeave_handleSelfMsg(omnetpp::cMessage* msg);
    void leaderLeave_BeaconFSM(BeaconVehicle *wsm = NULL);
    void leaderLeave_DataFSM(PlatoonMsg *wsm = NULL);

    // follower leave
    void followerLeave_handleSelfMsg(omnetpp::cMessage* msg);
    void followerLeave_BeaconFSM(BeaconVehicle *wsm = NULL);
    void followerLeave_DataFSM(PlatoonMsg *wsm = NULL);
    void performLaneChange();

    // dissolve
    void dissolve_handleSelfMsg(omnetpp::cMessage* msg);
    void dissolve_BeaconFSM(BeaconVehicle *wsm = NULL);
    void dissolve_DataFSM(PlatoonMsg *wsm = NULL);
};

}

#endif

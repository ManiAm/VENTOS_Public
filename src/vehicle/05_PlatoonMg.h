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

#include <04_PlatoonFormed.h>

namespace VENTOS {

class ApplVPlatoonMg : public ApplVPlatoonFormed
{
protected:
    // NED variables
    int maxPlnSize;
    int optPlnSize;

    bool adaptiveTG;
    double TP;
    double TG1;
    double TG2;

    bool entryEnabled;
    bool mergeEnabled;
    bool splitEnabled;
    bool followerLeaveEnabled;
    bool leaderLeaveEnabled;

    // Variables
    typedef enum states_num
    {
        state_idle,             // 0
        state_platoonLeader,    // 1
        state_platoonFollower,  // 2

        // entry states
        state_waitForLaneChange,

        // merge states
        state_sendMergeReq,
        state_waitForMergeReply,  // 5
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
        state_waitForAllAcks2,   // 20
        state_waitForCHANGEPL,
        state_sendingACK,
        state_waitForSplitDone,
        state_waitForGap,

        // leader leave
        state_sendVoteLeader,
        state_waitForVoteReply,
        state_splitCompleted,

        // follower leave
        state_sendLeaveReq,
        state_waitForLeaveReply,
        state_secondSplit,

        // dissolve
        state_sendDissolve,
        state_waitForDissolveAck,
    } states_num_t;

    states_num_t vehicleState = state_idle;

private:
    typedef ApplVPlatoonFormed super;

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

    // --[ split ]--
    std::string splittingVehicle = "";
    int splittingDepth = -1;
    std::string oldPlnID = "";
    int TotalPLSent = 0;
    int TotalACKsRx = 0;
    int splitCaller = -1;

    omnetpp::cMessage* plnTIMER4 = NULL;
    omnetpp::cMessage* plnTIMER5 = NULL;
    omnetpp::cMessage* plnTIMER6 = NULL;
    omnetpp::cMessage* plnTIMER7 = NULL;
    omnetpp::cMessage* plnTIMER8 = NULL;
    omnetpp::cMessage* plnTIMER8a = NULL;
    omnetpp::cMessage* mgrTIMER = NULL;

    // --[ leader leave ]--
    omnetpp::cMessage* plnTIMER9 = NULL;

    // --[ follower leave ]--
    int RemainingSplits = 0;

    omnetpp::cMessage* plnTIMER10 = NULL;
    omnetpp::cMessage* plnTIMER11 = NULL;

    // --[ dissolve ]--
    omnetpp::cMessage* plnTIMER12 = NULL;
    std::string lastVeh = "";

    typedef enum uCommand
    {
        MERGE_REQ, MERGE_ACCEPT, MERGE_REJECT, MERGE_DONE,
        SPLIT_REQ, SPLIT_ACCEPT, SPLIT_REJECT, SPLIT_DONE,
        CHANGE_PL, CHANGE_Tg,
        VOTE_LEADER, ELECTED_LEADER, DISSOLVE,
        LEAVE_REQ, LEAVE_ACCEPT, LEAVE_REJECT, GAP_CREATED,
        ACK,
    } uCommand_t;

public:
    ~ApplVPlatoonMg();
    virtual void initialize(int stage);
    virtual void finish();

protected:
    virtual void handleSelfMsg(omnetpp::cMessage*);
    virtual void handlePositionUpdate(cObject*);

    virtual void onBeaconVehicle(BeaconVehicle*);
    virtual void onBeaconRSU(BeaconRSU*);
    virtual void onPlatoonMsg(PlatoonMsg* wsm);

    void splitFromPlatoon(int);
    void leavePlatoon();
    void dissolvePlatoon();

private:
    PlatoonMsg* prepareData( std::string, uCommand_t, std::string, double db = -1, std::string str = "", std::deque<std::string> vec = std::deque<std::string>() );
    void updateColorDepth();

    // reporting to statistics
    void reportStateToStat();
    void reportCommandToStat(PlatoonMsg*);
    void reportManeuverToStat(std::string, std::string, std::string);
    const std::string stateToStr(int);
    const std::string uCommandToStr(int);

    // merge
    void merge_handleSelfMsg(omnetpp::cMessage* msg);
    void merge_BeaconFSM(BeaconVehicle *wsm = NULL);
    void merge_DataFSM(PlatoonMsg *wsm = NULL);
    void RemoveFollowerFromList_Merge(std::string);
    bool CatchUpDone();

    // split
    void split_handleSelfMsg(omnetpp::cMessage* msg);
    void split_BeaconFSM(BeaconVehicle *wsm = NULL);
    void split_DataFSM(PlatoonMsg *wsm = NULL);
    void splitMonitor();
    void RemoveFollowerFromList_Split(std::string);
    bool GapCreated();

    // common operations in maneuvers
    void common_handleSelfMsg(omnetpp::cMessage* msg);
    void common_BeaconFSM(BeaconVehicle *wsm = NULL);
    void common_DataFSM(PlatoonMsg *wsm = NULL);

    // entry
    void entry_handleSelfMsg(omnetpp::cMessage* msg);
    void entry_BeaconFSM(BeaconVehicle *wsm);
    void entry_DataFSM(PlatoonMsg *wsm = NULL);

    // leader leave
    void leaderLeave_handleSelfMsg(omnetpp::cMessage* msg);
    void leaderLeave_BeaconFSM(BeaconVehicle *wsm = NULL);
    void leaderLeave_DataFSM(PlatoonMsg *wsm = NULL);

    // follower leave
    void followerLeave_handleSelfMsg(omnetpp::cMessage* msg);
    void followerLeave_BeaconFSM(BeaconVehicle *wsm = NULL);
    void followerLeave_DataFSM(PlatoonMsg *wsm = NULL);

    // dissolve
    void dissolve_handleSelfMsg(omnetpp::cMessage* msg);
    void dissolve_BeaconFSM(BeaconVehicle *wsm = NULL);
    void dissolve_DataFSM(PlatoonMsg *wsm = NULL);
};

}

#endif

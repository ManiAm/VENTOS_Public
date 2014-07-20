
#ifndef APPLVPLATOONMG_H
#define APPLVPLATOONMG_H

#include "ApplV_05_PlatoonFormed.h"

class ApplVPlatoonMg : public ApplVPlatoonFormed
{
	public:
        ~ApplVPlatoonMg();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

		virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(PlatoonMsg* wsm);

        // NED variables
        int maxPlnSize;
        int optPlnSize;
        bool mergeEnabled;
        bool splitEnabled;

        // Variables
        enum states
        {
            state_idle,           // 0
            state_platoonLeader,  // 1
            state_platoonMember,  // 2

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
            state_sendSplitReq,     // 14
            state_waitForSplitReply,
            state_makeItFreeAgent,
            state_waitForAck,
            state_splitDone,
            state_changePL,
            state_waitForAllAcks2,  // 20
            state_waitForCHANGEPL,
            state_sendingACK,
            state_waitForSplitDone,
            state_waitForGap,
        };

        enum uCommands
        {
            MERGE_REQ,
            MERGE_ACCEPT,
            MERGE_REJECT,
            MERGE_DONE,

            CHANGE_PL,
            CHANGE_Tg,

            SPLIT_REQ,
            SPLIT_ACCEPT,
            SPLIT_REJECT,
            SPLIT_DONE,

            LEAVE_REQ,
            LEAVE_REJECT,

            VOTE_LEADER,
            ELECTED_LEADER,

            DISSOLVE,

            ACK,
        };

        states vehicleState;

	private:
        bool busy;

        // entry
        cMessage* entryManeuverEvt;
        double leastDistFront;
        double leastDistBack;
        NearestVehicle *leastFront;
        NearestVehicle *leastBack;

        cMessage* plnTIMER0;

        // merge
        string leadingPlnID;
        int leadingPlnDepth;
        deque<string> secondPlnMembersList;

        // split
        string splittingVehicle;
        int splittingDepth;
        string oldPlnID;

        cMessage* mgrTIMER;
        cMessage* plnTIMER1;
        cMessage* plnTIMER1a;
        cMessage* plnTIMER2;
        cMessage* plnTIMER3;

        cMessage* plnTIMER4;
        cMessage* plnTIMER5;
        cMessage* plnTIMER6;
        cMessage* plnTIMER7;
        cMessage* plnTIMER8;
        cMessage* plnTIMER8a;

	private:
        void Coordinator();
        PlatoonMsg* prepareData( string, uCommands, string, double db = -1, string str = "", deque<string> vec = deque<string>() );
        void printDataContent(PlatoonMsg*);
        void updateColorDepth();

        void reportStateToStat();
        void reportCommandToStat(PlatoonMsg*);
        const string stateToStr(int);
        const string uCommandToStr(int);

        void common_handleSelfMsg(cMessage* msg);
        void common_BeaconFSM(BeaconVehicle *wsm = NULL);
        void common_DataFSM(PlatoonMsg *wsm = NULL);

        void merge_handleSelfMsg(cMessage* msg);
        void merge_BeaconFSM(BeaconVehicle *wsm = NULL);
        void merge_DataFSM(PlatoonMsg *wsm = NULL);
        void RemoveFollowerFromList_Merge(string);
        bool CatchUpDone();

        void split_handleSelfMsg(cMessage* msg);
        void split_BeaconFSM(BeaconVehicle *wsm = NULL);
        void split_DataFSM(PlatoonMsg *wsm = NULL);
        void RemoveFollowerFromList_Split(string);
        bool GapDone();

        void entry_handleSelfMsg(cMessage* msg);
        void entry_BeaconFSM(BeaconVehicle *wsm);

        void followerLeave_FSM();
        void leaderLeave_FSM();
};

#endif

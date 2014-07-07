
#ifndef APPLVPLATOONMG_H
#define APPLVPLATOONMG_H

#include "ApplV_04_AID.h"

class ApplVPlatoonMg : public ApplV_AID
{
	public:
        ~ApplVPlatoonMg();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        // Methods
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

		virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(PlatoonMsg* wsm);

        // NED variables
        int maxPlnSize;
        int optPlnSize;

        // Variables
        enum states
        {
            state_idle,
            state_platoonLeader,
            state_platoonMember,

            // merge states
            state_sendMergeReq,
            state_waitForMergeReply,
            state_mergeAccepted,
            state_sendMergeDone,
            state_notifyFollowers,
            state_waitForAllAcks,
            state_sendMergeAccept,
            state_waitForMergeDone,
            state_mergeDone,





            state_wait_for_beacon,
            stateT_create_new_platoon,    //  transient
            state_ask_to_join,
            stateT_joining,               // transient
            stateT_handle_JOIN_request,   // transient

            // vehicle states (platoon leader/member leave)
            state_wait_for_new_PL,
            state_change_PL,
            state_parked,
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

	public:
        const char* stateToStr(int);
        const char* uCommandToStr(int);

	private:
        cMessage* entryManeuverEvt;
        bool busy;

        int leadingPlnDepth;
        string leadingPlnID;
        deque<string> secondPlnMembersList;

        cMessage* plnTIMER1;
        cMessage* plnTIMER2;
        cMessage* plnTIMER3;
        cMessage* plnTIMER4;

	private:
        // Methods
        PlatoonMsg* prepareData( string, uCommands, string, double db = -1, string str = "", deque<string> vec = deque<string>() );
        void printDataContent(PlatoonMsg*);
        void updateColor();

        void entry_handleSelfMsg(cMessage* msg);
        void entry_FSM();

        void merge_handleSelfMsg(cMessage* msg);
        void merge_BeaconFSM(BeaconVehicle *wsm = NULL);
        void merge_DataFSM(PlatoonMsg *wsm = NULL);
        void RemoveFollowerFromList(string);

        void split_handleSelfMsg(cMessage* msg);
        void split_BeaconFSM(BeaconVehicle *wsm = NULL);
        void split_DataFSM(PlatoonMsg *wsm = NULL);

        void followerLeave_FSM();
        void leaderLeave_FSM();
};

#endif

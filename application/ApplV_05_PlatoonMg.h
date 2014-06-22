
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
        int maxPlatoonSize;

        // Variables
        enum messages
        {
            // vehicle states (platoon formation)
            state_idle,                        // 0
            state_wait_for_beacon,             // 1
            stateT_create_new_platoon,         // 2  transient
            state_ask_to_join,                 // 3
            stateT_joining,                    // 4  transient
            state_platoonLeader,               // 5
            state_platoonMember,               // 6
            stateT_handle_JOIN_request,        // 7  transient

            // vehicle states (platoon leader/member leave)
            state_wait_for_new_PL,
            state_change_PL,
            state_parked,

            // platoon formation messages
            JOIN_request,
            JOIN_ACCEPT_response,
            JOIN_REJECT_response,
            CHANGE_Tg,

            // platoon leader/member leave messages
            NEW_LEADER_request,
            NEW_LEADER_ACCEPT_response,
            CHANGE_PL,
        };

        messages vehicleState;
        cMessage* EntryManeuverEvt;

	private:
        // Methods
        void entryManeuver();
        void entryFSM();

        void mergeManeuver();
        void mergeFSM();

        void splitManeuver();
        void splitFSM();

        void platoonLeaderLeaveManeuver();
        void platoonLeaderLeaveFSM();

        void platoonMemberLeaveManeuver();
        void platoonMemberLeaveFSM();
};

#endif

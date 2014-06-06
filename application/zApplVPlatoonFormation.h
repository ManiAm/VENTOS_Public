
#ifndef ApplVPlatoonFORMATION_H
#define ApplVPlatoonFORMATION_H

#include "ApplV_06_Manager.h"


class ApplVPlatoonFormation : public ApplVManager
{
	public:
        ~ApplVPlatoonFormation();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        virtual void handleLowerMsg(cMessage* msg);
        virtual void handleSelfMsg(cMessage* msg);
        virtual void handlePositionUpdate(cObject* obj);

		virtual void onBeaconVehicle(BeaconVehicle* wsm);
		virtual void onData(PlatoonMsg* wsm);

		PlatoonMsg* prepareData( std::string, int, std::string, double db = -1, std::string str = "", std::deque<std::string> vec = std::deque<std::string>() );
        void printDataContent(PlatoonMsg*);

		void FSMchangeState();

	protected:
        // NED variables
        bool platoonFormation;
        int maxPlatoonSize;
        double timer1Value;
        double timer2Value;

        int dataLengthBits;
        bool dataOnSch;
        int dataPriority;

        // Class variables
        int platoonSize;
        BeaconVehicle* myLeadingBeacon;  // a copy of leading beacon
        cMessage* TIMER1;
        cMessage* TIMER2;

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

            // timers
            timer_wait_for_beacon_from_leading,
            timer_wait_for_JOIN_response,
            timer_PL_leave,
            timer_PM_leave,
            timer_wait_for_newPL_response
        };

        messages vehicleState;

        // colors of platoon members
        int *pickColor;

        std::deque<std::string> queue;
};

#endif

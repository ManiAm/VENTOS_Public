
#ifndef ApplVPlatoon_H
#define ApplVPlatoon_H

#include "ApplVPlatoon.h"


class ApplVPlatoonFormation : public ApplVPlatoon
{
	public:
        ~ApplVPlatoonFormation();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        virtual void handleLowerMsg(cMessage* msg);
        virtual void handleSelfMsg(cMessage* msg);
        virtual void handlePositionUpdate(cObject* obj);

		virtual void onBeacon(Beacon* wsm);
		virtual void onData(PlatoonMsg* wsm);

		PlatoonMsg* prepareData(std::string, int, std::string, int);
        void printDataContent(PlatoonMsg*);

		void FSMchangeState();

	private:
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
        Beacon* myLeadingBeacon;  // a copy of leading beacon
        cMessage* TIMER1;
        cMessage* TIMER2;

        enum messages
        {
            // platoon states
            state_idle,                        // 0
            state_wait_for_beacon,             // 1
            stateT_create_new_platoon,         // 2  transient
            state_ask_to_join,                 // 3
            stateT_joining,                    // 4  transient
            state_platoonLeader,               // 5
            state_platoonMember,               // 6
            stateT_handle_JOIN_request,        // 7  transient
            stateT_handle_CHANGE_Tg_request,   // 8  transient

            // platoon msg
            JOIN_request,
            JOIN_ACCEPT_response,
            JOIN_REJECT_response,
            CHANGE_Tg,
            LEAVE_request,

            // timers
            timer_wait_for_beacon_from_leading,
            timer_wait_for_JOIN_response
        };

        messages vehicleState;

        // colors of platoon members
        int *pickColor;
};

#endif


#ifndef ApplVPlatoon_H
#define ApplVPlatoon_H

#include "ApplVBeaconPlatoonLeader.h"

class ApplVPlatoon : public ApplVBeaconPlatoonLeader
{
	public:
        ~ApplVPlatoon();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        virtual void handleLowerMsg(cMessage* msg);
        virtual void handleSelfMsg(cMessage* msg);
        virtual void handlePositionUpdate(cObject* obj);

		virtual void onBeacon(WaveShortMessage* wsm);
		virtual void onData(WaveShortMessage* wsm);

		void FSMchangeState();
        void sendMessage(std::string);

	private:
        // NED variables
        bool platoonFormation;
        int maxPlatoonSize;
        int dataLengthBits;
        bool dataOnSch;
        int dataPriority;

        // Class variables
        int platoonSize;

        simtime_t lastDroveAt;
        bool sentMessage;

        enum PlatoonStates
        {
            state_idle,
            state_wait_for_beacon,
            stateT_create_new_platoon,   // transient
            state_ask_to_join,
            stateT_joining,              // transient
            state_platoonLeader,
            state_platoonMember,
            stateT_handle_JOIN_request,        // transient
            stateT_handle_CHANGE_Tg_request,   // transient

            timer_
        };

        PlatoonStates vehicleState;
};

#endif

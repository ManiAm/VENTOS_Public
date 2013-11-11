
#ifndef ApplVPlatoon_H
#define ApplVPlatoon_H

#include "ApplVBeacon.h"

class ApplVPlatoon : public ApplVBeacon
{
	public:
        ~ApplVPlatoon();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        /** @brief handle messages from below */
        virtual void handleLowerMsg(cMessage* msg);
        /** @brief handle self messages */
        virtual void handleSelfMsg(cMessage* msg);
        /** @brief handle position updates */
        virtual void handlePositionUpdate(cObject* obj);

		virtual void onBeacon(WaveShortMessage* wsm);
		virtual void onData(WaveShortMessage* wsm);

        WaveShortMessage* fillBeaconPlatoon(WaveShortMessage *);

	private:
        bool isPlatoonLeader;
        long platoonID;
        std::string platoonLeaderID;
        bool one_vehicle_look_ahead;
};

#endif

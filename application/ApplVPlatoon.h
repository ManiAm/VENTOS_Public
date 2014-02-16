
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

        void sendMessage(std::string);

	private:
        // NED variables
        bool platoonFormation;
        int dataLengthBits;
        bool dataOnSch;
        int dataPriority;

        // Class variables
        int platoonSize;
        int maxPlatoonSize;
        simtime_t lastDroveAt;
        bool sentMessage;
};

#endif

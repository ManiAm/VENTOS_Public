
#ifndef ApplVBeaconPLATOONLEADER_H
#define ApplVBeaconPLATOONLEADER_H

#include "ApplVBeacon.h"

class ApplVBeaconPlatoonLeader : public ApplVBeacon
{
	public:
        ~ApplVBeaconPlatoonLeader();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        // NED variables
        bool one_vehicle_look_ahead;
        std::string platoonID;

        // Class variables
        int myPlatoonDepth;

        // Methods
        virtual void handleLowerMsg(cMessage*);
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

		virtual void onBeacon(Beacon*);

		Beacon* fillBeaconPlatoon(Beacon *);

	private:
        bool isBeaconFromPlatoonLeader(Beacon*);
};

#endif

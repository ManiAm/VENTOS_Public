
#ifndef ApplVPLATOON_H
#define ApplVPLATOON_H

#include "ApplVBeacon.h"

class ApplVPlatoon : public ApplVBeacon
{
	public:
        ~ApplVPlatoon();
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

        bool isBeaconFromMyPlatoonLeader(Beacon*);
        bool isBeaconFromLeading(Beacon*);
        double getGap(std::string);
};

#endif

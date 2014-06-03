
#ifndef ApplVAID_H
#define ApplVAID_H

#include "03ApplVBeacon.h"

class ApplV_AID : public ApplVBeacon
{
	public:
        ~ApplV_AID();
		virtual void initialize(int stage);
        virtual void finish();

	protected:

        // NED
        bool AID;

        int dataLengthBits;
        bool dataOnSch;
        int dataPriority;

        // Methods
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

		virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(PlatoonMsg* wsm);

        // class variables
        std::string lastLane;
        std::deque<std::string> laneChanges;
};

#endif

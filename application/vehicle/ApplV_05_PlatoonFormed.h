
#ifndef APPLVPLATOONFORMED_H
#define APPLVPLATOONFORMED_H

#include "ApplV_04_AID.h"

namespace VENTOS {

class ApplVPlatoonFormed : public ApplV_AID
{
	public:
        ~ApplVPlatoonFormed();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        // Methods
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

		virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(PlatoonMsg* wsm);

	protected:
        int plnMode;
        string preDefinedPlatoonID;
};
}

#endif

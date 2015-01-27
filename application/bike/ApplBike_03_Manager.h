
#ifndef ApplBIKEMANAGER_H
#define ApplBIKEMANAGER_H

#include "ApplBike_02_Beacon.h"

namespace VENTOS {

class ApplBikeManager : public ApplBikeBeacon
{
	public:
        ~ApplBikeManager();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        // Methods
        virtual void handleLowerMsg(cMessage*);
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

		virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(PlatoonMsg* wsm);

	protected:
        // NED variable
        bool SUMOvehicleDebug;
};

}

#endif

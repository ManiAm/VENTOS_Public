
#ifndef ApplPEDMANAGER_H
#define ApplPEDMANAGER_H

#include "ApplPed_02_Beacon.h"

namespace VENTOS {

class ApplPedManager : public ApplPedBeacon
{
	public:
        ~ApplPedManager();
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


#ifndef ApplVMANAGER_H
#define ApplVMANAGER_H

#include "ApplV_07_Coordinator.h"

namespace VENTOS {

class ApplVManager : public ApplVCoordinator
{
	public:
        ~ApplVManager();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        // Methods
        virtual void handleLowerMsg(cMessage*);
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

        virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconPedestrian(BeaconPedestrian*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(PlatoonMsg* wsm);

	private:
        bool dropBeacon(double time, string vehicle, double plr);
        void reportDropToStatistics(BeaconVehicle* wsm);

	protected:
        // NED variables (packet loss ratio)
        double droppT;
        string droppV;
        double plr;

        // NED variable
        int controllerType;
        bool degradeToACC;
        bool SUMOvehicleDebug;

        // NED variables (measurement error)
        bool measurementError;
        double errorGap;
        double errorRelSpeed;

        long BeaconVehCount;
        long BeaconVehDropped;
        long BeaconRSUCount;
        long PlatoonCount;
        long BeaconPedCount;
};

}

#endif

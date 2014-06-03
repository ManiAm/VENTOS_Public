
#ifndef ApplVMANAGER_H
#define ApplVMANAGER_H

#include "ApplVBeacon.h"

class ApplVManager : public ApplVBeacon
{
	public:
        ~ApplVManager();
		virtual void initialize(int stage);
        virtual void finish();

	protected:

        // NED variables
        double sonarDist;

        // NED variables (packet loss ratio)
        double droppT;
        std::string droppV;
        double plr;

        // NED variable
        bool SUMOvehicleDebug;
        bool modeSwitch;

        // NED variables (measurement error)
        bool measurementError;
        double errorGap;
        double errorRelSpeed;

        // NED variables
        int mode;
        bool one_vehicle_look_ahead;

        // NED
        std::string preDefinedPlatoonID;

        // Methods
        virtual void handleLowerMsg(cMessage*);
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

		virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onData(PlatoonMsg* wsm);

        bool dropBeacon(double time, std::string vehicle, double plr);
        void reportDropToStatistics(BeaconVehicle* wsm);
        bool isBeaconFromLeading(BeaconVehicle*);
        bool isBeaconFromMyPlatoonLeader(BeaconVehicle*);
};

#endif


#ifndef ApplVMANAGER_H
#define ApplVMANAGER_H

#include "ApplV_04_PlatoonMg.h"

class ApplVManager : public ApplVPlatoonMg
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
        bool one_vehicle_look_ahead;

        // Methods
        virtual void handleLowerMsg(cMessage*);
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

		virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(PlatoonMsg* wsm);

	private:

        bool dropBeacon(double time, std::string vehicle, double plr);
        void reportDropToStatistics(BeaconVehicle* wsm);
        bool isBeaconFromLeading(BeaconVehicle*);
        bool isBeaconFromMyPlatoonLeader(BeaconVehicle*);
};

#endif

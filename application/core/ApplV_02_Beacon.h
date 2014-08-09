
#ifndef ApplVBeacon_H
#define ApplVBeacon_H

#include "ApplV_01_Base.h"

namespace VENTOS {

class ApplVBeacon : public ApplVBase
{
	public:
        ~ApplVBeacon();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        // NED
        bool VANETenabled;
        int controlMode;
        int plnMode;
        double sonarDist;

        // NED variables (beaconing parameters)
        bool sendBeacons;
		double beaconInterval;
		double maxOffset;
        int beaconLengthBits;
        int beaconPriority;

        // NED variables (data message parameters)
        int dataLengthBits;
        bool dataOnSch;
        int dataPriority;

        // Class variables
        simtime_t individualOffset;
        cMessage* VehicleBeaconEvt;
        bool pauseBeaconing;

        string plnID;
        int myPlnDepth;
        int plnSize;
        deque<string> plnMembersList;

protected:
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

        BeaconVehicle* prepareBeacon();
        void printBeaconContent(BeaconVehicle*);

        bool isBeaconFromLeading(BeaconVehicle*);
        bool isBeaconFromMyPlatoonLeader(BeaconVehicle*);
};
}

#endif

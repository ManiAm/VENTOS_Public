
#ifndef ApplBIKEBeacon_H
#define ApplBIKEBeacon_H

#include "ApplBike_01_Base.h"

namespace VENTOS {

class ApplBikeBeacon : public ApplBikeBase
{
	public:
        ~ApplBikeBeacon();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        // NED
        bool VANETenabled;

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
        cMessage* BicycleBeaconEvt;

protected:
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

        BeaconBicycle* prepareBeacon();
        void printBeaconContent(BeaconBicycle*);
};

}

#endif

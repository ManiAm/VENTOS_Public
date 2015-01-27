
#ifndef ApplPEDBeacon_H
#define ApplPEDBeacon_H

#include "ApplPed_01_Base.h"

namespace VENTOS {

class ApplPedBeacon : public ApplPedBase
{
	public:
        ~ApplPedBeacon();
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
        cMessage* PedestrianBeaconEvt;

protected:
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

        BeaconPedestrian* prepareBeacon();
        void printBeaconContent(BeaconPedestrian*);
};

}

#endif

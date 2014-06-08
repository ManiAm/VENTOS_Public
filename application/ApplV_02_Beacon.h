
#ifndef ApplVBeacon_H
#define ApplVBeacon_H

#include "ApplV_01_Base.h"

class ApplVBeacon : public ApplVBase
{
	public:
        ~ApplVBeacon();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        // NED
        bool VANETenabled;
        int mode;
        string preDefinedPlatoonID;

        // NED variables (beaconing parameters)
        bool sendBeacons;
		double beaconInterval;
		double maxOffset;
        int beaconLengthBits;
        int beaconPriority;

        // Class variables
        simtime_t individualOffset;
        cMessage* sendBeaconEvt;
        bool pauseBeaconing;

        string platoonID;
        int myPlatoonDepth;
        int platoonSize;
        deque<string> queue;

        // Methods
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

        BeaconVehicle* prepareBeacon();
        void printBeaconContent(BeaconVehicle*);
};

#endif

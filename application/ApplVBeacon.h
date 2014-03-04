
#ifndef ApplVBeacon_H
#define ApplVBeacon_H

#include "ApplVBase.h"

class ApplVBeacon : public ApplVBase
{
	public:
        ~ApplVBeacon();
		virtual void initialize(int stage);
        virtual void finish();

	protected:
        // NED variables (beaconing parameters)
        bool sendBeacons;
		double beaconInterval;
		double maxOffset;
        int beaconLengthBits;
        int beaconPriority;

        // NED variables (packet loss ratio)
        double droppT;
        std::string droppV;
        double plr;

        // NED variable
        bool modeSwitch;

        // Class variables
        simtime_t individualOffset;
        cMessage* sendBeaconEvt;

        // Methods
        virtual void handleLowerMsg(cMessage*);
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

        virtual void onBeacon(Beacon*);

		Beacon* prepareBeacon();
        void printBeaconContent(Beacon*);
        bool dropBeacon(double, std::string, double);

	private:
        bool isBeaconFromLeading(Beacon*);
        double getGap(std::string);
};

#endif

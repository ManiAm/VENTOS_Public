
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

		virtual void onBeacon(WaveShortMessage*);
		virtual void onData(WaveShortMessage*);

        WaveShortMessage* prepareBeacon(std::string, int, t_channel, int, int, int serial=0);
        void printBeaconContent(WaveShortMessage*);
        bool dropBeacon(double, std::string, double);

	private:
        double getGap(std::string);
        bool isBeaconFromLeading(WaveShortMessage*);
};

#endif


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
		AnnotationManager* annotations;
		simtime_t lastDroveAt;
		bool sentMessage;

        bool modeSwitch;
        double droppT;
        std::string droppV;
        double plr;

		bool sendBeacons;
		double beaconInterval;
		double maxOffset;
        int beaconLengthBits;
        int beaconPriority;

        simtime_t individualOffset;

        cMessage* sendBeaconEvt;

	protected:
        /** @brief handle messages from below */
        virtual void handleLowerMsg(cMessage*);
        /** @brief handle self messages */
        virtual void handleSelfMsg(cMessage*);
        /** @brief handle position updates */
        virtual void handlePositionUpdate(cObject*);

		virtual void onBeacon(WaveShortMessage*);
		virtual void onData(WaveShortMessage*);

        WaveShortMessage* prepareBeacon(std::string, int, t_channel, int, int, int serial=0);
        void printBeaconContent(WaveShortMessage*);

		void sendMessage(std::string);
        virtual void sendWSM(WaveShortMessage*);

	private:
        bool dropBeacon(double, std::string, double);
        double  getGap(std::string);
        bool isBeaconFromLeading(WaveShortMessage*);
};

#endif

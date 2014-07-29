
#ifndef APPLRSU_H_
#define APPLRSU_H_

#include <BaseApplLayer.h>
#include "Appl.h"

#include <map>
#include <Consts80211p.h>
#include <Mac80211Pkt_m.h>
#include <ChannelAccess.h>
#include <WaveAppToMac1609_4Interface.h>
#include "Global_01_TraCI_Extend.h"
#include "mobility/traci/TraCIColor.h"
#include "Global_07_RSUAdd.h"

using namespace std;


class ApplRSUBase : public BaseApplLayer
{
	public:
		~ApplRSUBase();
		virtual void initialize(int stage);
		virtual void finish();
		virtual void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj);

		enum WaveApplMessageKinds
		{
			SERVICE_PROVIDER = LAST_BASE_APPL_MESSAGE_KIND,
			KIND_TIMER
		};

	protected:
		static const simsignalwrap_t mobilityStateChangedSignal;

		virtual void handleSelfMsg(cMessage* msg);

        BeaconRSU* prepareBeacon();
        void printBeaconContent(BeaconRSU*);

	protected:
		// NED variables
	    cModule *nodePtr;   // pointer to the Node
        WaveAppToMac1609_4Interface* myMac;
        mutable TraCI_Extend* TraCI;
        RSUAdd *RSUAddPtr;

        // NED variables (beaconing parameters)
        bool sendBeacons;
        double beaconInterval;
        double maxOffset;
        int beaconLengthBits;
        int beaconPriority;

        // Class variables
        int myId;
		const char *myFullId;
        simtime_t individualOffset;
        cMessage* RSUBeaconEvt;
};

#endif

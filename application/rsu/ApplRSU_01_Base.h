
#ifndef APPLRSU_H_
#define APPLRSU_H_

#include <BaseApplLayer.h>
#include <WaveAppToMac1609_4Interface.h>
#include "TraCI_Extend.h"

using namespace std;

namespace VENTOS {

class TraCI_Extend;
class RSUAdd;

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
		Coord *getRSUsCoord(unsigned int);

		virtual void handleSelfMsg(cMessage* msg);

        BeaconRSU* prepareBeacon();
        void printBeaconContent(BeaconRSU*);

	protected:
		// NED variables
	    cModule *nodePtr;   // pointer to the Node
        WaveAppToMac1609_4Interface* myMac;
        mutable TraCI_Extend* TraCI;

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

}

#endif

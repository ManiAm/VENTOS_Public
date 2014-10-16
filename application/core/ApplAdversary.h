
#ifndef APPLADVERSARY_H_
#define APPLADVERSARY_H_

#include <BaseApplLayer.h>
#include <ChannelAccess.h>
#include <WaveAppToMac1609_4Interface.h>

#include "Appl.h"
#include "Global_01_TraCI_Extend.h"

using namespace std;

namespace VENTOS {

class ApplAdversary : public BaseApplLayer
{
	public:
		~ApplAdversary();
		virtual void initialize(int stage);
		virtual void finish();
		virtual void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj);

		enum WaveApplMessageKinds
		{
			SERVICE_PROVIDER = LAST_BASE_APPL_MESSAGE_KIND,
			KIND_TIMER
		};

	protected:
		virtual void handleLowerMsg(cMessage* msg);
		virtual void handleSelfMsg(cMessage* msg);
		virtual void handlePositionUpdate(cObject* obj);

	private:
        void DoFalsificationAttack(BeaconVehicle * wsm);
        void DoReplayAttack(BeaconVehicle * wsm);
        void DoJammingAttack();

        DummyMsg* CreateDummyMessage();

	protected:
		// NED variables
	    cModule *nodePtr;   // pointer to the Node
        WaveAppToMac1609_4Interface* myMac;
        mutable TraCI_Extend* TraCI;

        // NED variables
        double AttackT;
        bool falsificationAttack;
        bool replayAttack;
        bool jammingAttack;

        // Class variables
        static const simsignalwrap_t mobilityStateChangedSignal;
        int myId;
		const char *myFullId;
        Coord curPosition;  // current position from mobility module (not from sumo)
        cMessage* JammingEvt;
};

}

#endif

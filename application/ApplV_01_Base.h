
#ifndef APPLVBASE_H_
#define APPLVBASE_H_

#include <BaseApplLayer.h>
#include "Appl.h"

#include <map>
#include <Consts80211p.h>
#include <Mac80211Pkt_m.h>
#include <msg/Messages_m.h>
#include <ChannelAccess.h>
#include <WaveAppToMac1609_4Interface.h>
#include "Global_01_TraCI_Extend.h"
#include "mobility/traci/TraCIColor.h"

using namespace std;


class ApplVBase : public BaseApplLayer
{
	public:
		~ApplVBase();
		virtual void initialize(int stage);
		virtual void finish();
		virtual void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj);

		enum WaveApplMessageKinds
		{
			SERVICE_PROVIDER = LAST_BASE_APPL_MESSAGE_KIND,
			SEND_BEACON_EVT,
			SEND_SYSTEMMSG_EVT,
		};

	protected:
		static const simsignalwrap_t mobilityStateChangedSignal;

		virtual void handleSelfMsg(cMessage* msg);
		virtual void handlePositionUpdate(cObject* obj);

	protected:
		// NED variables
	    cModule *nodePtr;   // pointer to the Node
        WaveAppToMac1609_4Interface* myMac;
        TraCIMobility* TraCI_Mobility;
        mutable TraCI_Extend* TraCI;
        AnnotationManager* annotations;

        // Class variables
        int myId;
		const char *myFullId;
	    string SUMOvID;
        string SUMOvType;
        Coord curPosition;  // current position from mobility module (not from sumo)
};

#endif


#ifndef APPLVBASE_H_
#define APPLVBASE_H_

#include <map>
#include <BaseApplLayer.h>
#include <Consts80211p.h>
#include <Mac80211Pkt_m.h>
#include <msg/Messages_m.h>
#include <ChannelAccess.h>
#include <WaveAppToMac1609_4Interface.h>
#include "15TraCI_Extend.h"
#include "mobility/traci/TraCIColor.h"
#include "01ExtraClasses.h"


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
			SEND_BEACON_EVT
		};

	protected:
		static const simsignalwrap_t mobilityStateChangedSignal;

		virtual void handleSelfMsg(cMessage* msg);
		virtual void handlePositionUpdate(cObject* obj);

		bool isCACCvehicle();

	protected:
		// NED variables
	    cModule *nodePtr;   // pointer to the Node
        WaveAppToMac1609_4Interface* myMac;
        TraCIMobility* traci;
        mutable TraCI_Extend* manager;
        AnnotationManager* annotations;

        // Class variables
        int myId;
		const char *myFullId;
	    std::string SUMOvID;
        std::string SUMOvType;
        Coord curPosition;  // current position from mobility module (not from sumo)
};

#endif

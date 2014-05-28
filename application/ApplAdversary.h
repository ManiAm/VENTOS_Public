
#ifndef APPLADVERSARY_H_
#define APPLADVERSARY_H_

#include <map>
#include <BaseApplLayer.h>
#include <Consts80211p.h>
#include <Mac80211Pkt_m.h>
#include <msg/Messages_m.h>
#include <ChannelAccess.h>
#include <WaveAppToMac1609_4Interface.h>
#include "TraCI_Extend.h"
#include "mobility/traci/TraCIColor.h"

#include "ExtraClasses.h"


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
			SEND_BEACON_EVT
		};

	protected:
		static const simsignalwrap_t mobilityStateChangedSignal;

		/** @brief handle messages from below */
		virtual void handleLowerMsg(cMessage* msg);
		/** @brief handle self messages */
		virtual void handleSelfMsg(cMessage* msg);
		/** @brief handle position updates */
		virtual void handlePositionUpdate(cObject* obj);

		virtual void sendWSM(Beacon * wsm);

	protected:
		// NED variables
	    cModule *nodePtr;   // pointer to the Node
        WaveAppToMac1609_4Interface* myMac;
        mutable TraCI_Extend* manager;
        bool on;

        // Class variables
        int myId;
		const char *myFullId;
        Coord curPosition;  // current position from mobility module (not from sumo)
};

#endif

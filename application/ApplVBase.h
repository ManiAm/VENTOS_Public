
#ifndef APPLVBASE_H_
#define APPLVBASE_H_

#include <map>
#include <BaseApplLayer.h>
#include <Consts80211p.h>
#include <Mac80211Pkt_m.h>
#include <msg/WaveShortMessage_m.h>
#include <ChannelAccess.h>
#include <WaveAppToMac1609_4Interface.h>
#include "MyTraCI.h"

#ifndef DBG
#define DBG EV
#endif

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

		/** @brief handle messages from below */
		virtual void handleLowerMsg(cMessage* msg);
		/** @brief handle self messages */
		virtual void handleSelfMsg(cMessage* msg);
		/** @brief handle position updates */
		virtual void handlePositionUpdate(cObject* obj);

		virtual WaveShortMessage* prepareData(std::string name, int dataLengthBits, t_channel channel, int priority, int rcvId, int serial=0);
		virtual void sendWSM(WaveShortMessage* wsm);

	protected:
        WaveAppToMac1609_4Interface* myMac;

        int mySCH;
		int myId;
        Coord curPosition;

        TraCIMobility* traci;
        mutable MyTraCI* manager;

		std::string SUMOvID;
        std::string SUMOvType;

        bool sendData;
        int dataLengthBits;
        bool dataOnSch;
        int dataPriority;
};

#endif

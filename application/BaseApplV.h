
#ifndef BASEAPPLV_H_
#define BASEAPPLV_H_

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
//#define DBG std::cerr << "[" << simTime().raw() << "] " << getParentModule()->getFullPath() << " "

class BaseApplV : public BaseApplLayer
{
	public:
		~BaseApplV();
		virtual void initialize(int stage);
		virtual void finish();

		virtual  void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj);

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

		virtual WaveShortMessage* prepareWSM(std::string name, int dataLengthBits, t_channel channel, int priority, int rcvId, int serial=0);
		virtual void sendWSM(WaveShortMessage* wsm);
		virtual void onBeacon(WaveShortMessage* wsm) = 0;
		virtual void onData(WaveShortMessage* wsm) = 0;

		virtual void handlePositionUpdate(cObject* obj);
		void printMsg(WaveShortMessage*);

	protected:
		int beaconLengthBits;
		int beaconPriority;
		bool sendData;
		bool sendBeacons;
		simtime_t individualOffset;
		int dataLengthBits;
		bool dataOnSch;
		int dataPriority;
		Coord curPosition;
		int mySCH;
		int myId;

		// vehicle id in sumo
		std::string SUMOvID;
		// vehicle type in sumo
        std::string SUMOvType;

		cMessage* sendBeaconEvt;

        TraCIMobility* traci;
        mutable MyTraCI* manager;

		WaveAppToMac1609_4Interface* myMac;
};

#endif /* BASEWAVEAPPLLAYER_H_ */


#ifndef ApplV_H
#define ApplV_H

#include "BaseWaveApplLayer.h"
#include "mobility/traci/TraCIMobility.h"

class ApplV : public BaseWaveApplLayer {
	public:
		virtual void initialize(int stage);

	protected:
		TraCIMobility* traci;
		AnnotationManager* annotations;
		simtime_t lastDroveAt;
		bool sentMessage;

	protected:
		virtual void onBeacon(WaveShortMessage* wsm);
		virtual void onData(WaveShortMessage* wsm);
		void sendMessage(std::string blockedRoadId);
		virtual void handlePositionUpdate(cObject* obj);
};

#endif

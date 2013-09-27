
#ifndef ApplV_H
#define ApplV_H

#include "BaseApplV.h"

class ApplV : public BaseApplV
{
	public:
		virtual void initialize(int stage);

	protected:
		AnnotationManager* annotations;
		simtime_t lastDroveAt;
		bool sentMessage;

	protected:
		virtual void onBeacon(WaveShortMessage* wsm);
		virtual void onData(WaveShortMessage* wsm);
		void sendMessage(std::string blockedRoadId);
		virtual void handlePositionUpdate(cObject* obj);

        std::string  getLeader();
        double  getGap(std::string, std::string);
        bool isBeaconFromLeader(WaveShortMessage*);
        void updateParamsSumo(WaveShortMessage*);
};

#endif

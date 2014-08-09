
#ifndef ApplVAID_H
#define ApplVAID_H

#include "router/ApplVSystem.h"

namespace VENTOS {

class ApplV_AID : public ApplVSystem
{
	public:
        ~ApplV_AID();
		virtual void initialize(int stage);
        virtual void finish();

	protected:

        // NED
        bool AID;

        // Methods
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePositionUpdate(cObject*);

		virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(PlatoonMsg* wsm);

        LaneChangeMsg* prepareData(string, deque<string>);
        void printDataContent(LaneChangeMsg*);

        // class variables
        string fromLane;
        string toLane;
        double fromX;
        double toX;

        deque<string> laneChanges;
};
}

#endif

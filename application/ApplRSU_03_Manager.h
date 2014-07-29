
#ifndef APPLRSUMANAGER_H_
#define APPLRSUMANAGER_H_

#include <ApplRSU_02_AID.h>

namespace VENTOS {

class ApplRSUManager : public ApplRSUAID
{
	public:
		~ApplRSUManager();
		virtual void initialize(int stage);
		virtual void finish();

	protected:
		virtual void handleLowerMsg(cMessage* msg);
		virtual void handleSelfMsg(cMessage* msg);

        virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(LaneChangeMsg*);
};
}

#endif

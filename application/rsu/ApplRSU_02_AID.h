
#ifndef APPLRSUAID_H_
#define APPLRSUAID_H_

#include "ApplRSU_01_Base.h"

namespace VENTOS {

class ApplRSUAID : public ApplRSUBase
{
	public:
		~ApplRSUAID();
		virtual void initialize(int stage);
		virtual void finish();

		// are declared public to be accessible by Statistics
        static MatrixXi tableCount;
        static MatrixXd tableProb;

	protected:
        virtual void handleSelfMsg(cMessage* msg);

        virtual void onBeaconVehicle(BeaconVehicle*);
        virtual void onBeaconBicycle(BeaconBicycle*);
        virtual void onBeaconPedestrian(BeaconPedestrian*);
        virtual void onBeaconRSU(BeaconRSU*);
        virtual void onData(LaneChangeMsg*);
};

}

#endif

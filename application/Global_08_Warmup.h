
#ifndef WARMPUP
#define WARMPUP

#include "Global_01_TraCI_Extend.h"
#include "Global_06_VehicleAdd.h"
#include "Global_09_SpeedProfile.h"

namespace VENTOS {

class Warmup : public BaseModule
{
	public:
		virtual ~Warmup();
		virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
		virtual void finish();

	public:
		bool DoWarmup();

	private:

        // NED variables
        cModule *nodePtr;       // pointer to the Node module
        TraCI_Extend *TraCI;  // pointer to the TraCI module
        VehicleAdd *AddVehiclePtr;
        bool on;
        string laneId;
        double stopPosition;  // the position that first vehicle should stop waiting for others
        double waitingTime;
        int totalVehicles;

        // class variables
        double startTime;     // the time that Warmup starts
        bool IsWarmUpFinished;
        cMessage* warmupFinish;

        // methods
        bool warmUpFinished();
};
}

#endif

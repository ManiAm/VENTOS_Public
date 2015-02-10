
#ifndef WARMPUP
#define WARMPUP

#include "TraCI_Extend.h"
#include "AddVehicle.h"
#include "VehicleSpeedProfile.h"

namespace VENTOS {

class Warmup : public BaseModule
{
	public:
		virtual ~Warmup();
		virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
		virtual void finish();
	    virtual void receiveSignal(cComponent *, simsignal_t, long);

	private:

        // NED variables
        cModule *nodePtr;       // pointer to the Node module
        TraCI_Extend *TraCI;  // pointer to the TraCI module
        SpeedProfile *SpeedProfilePtr;
        AddVehicle *AddVehiclePtr;
        simsignal_t Signal_executeEachTS;
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
        bool DoWarmup();
        bool warmUpFinished();
};

}

#endif

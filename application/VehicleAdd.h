
#ifndef VEHICLEADD
#define VEHICLEADD

#include <omnetpp.h>
#include "mobility/traci/TraCIScenarioManagerLaunchd.h"
#include "mobility/traci/TraCIMobility.h"
#include "mobility/traci/TraCIConstants.h"
#include "TraCI_Extend.h"


class VehicleAdd : public cSimpleModule
{
	public:
		virtual ~VehicleAdd();
		virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
		virtual void finish();

	public:
        void Add();

	private:

        // NED variables
        cModule *nodePtr;   // pointer to the Node
        TraCI_Extend *manager;  // pointer to the TraCI module
        bool on;
        int mode;
        int platoonSize;
        int platoonNumber;
	    int totalVehicles;

	    // methods
	    void Scenario1();
        void Scenario2();
        void Scenario3();
};


#endif

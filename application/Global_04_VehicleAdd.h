
#ifndef VEHICLEADD
#define VEHICLEADD

#include <omnetpp.h>
#include "mobility/traci/TraCIScenarioManagerLaunchd.h"
#include "mobility/traci/TraCIMobility.h"
#include "mobility/traci/TraCIConstants.h"
#include "Global_01_TraCI_Extend.h"
#include "ApplV_05_Manager.h"
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;


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
        TraCI_Extend *TraCI;  // pointer to the TraCI module
        bool on;
        int mode;
        int platoonSize;
        int platoonNumber;
	    int totalVehicles;

	    std::string xmlFileName;

	    // methods
	    void Scenario1();
        void Scenario2();
        void Scenario3();
        void Scenario4();
        void Scenario5();
};


#endif

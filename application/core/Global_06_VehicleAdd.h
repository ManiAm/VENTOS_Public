
#ifndef VEHICLEADD
#define VEHICLEADD

#include "Global_01_TraCI_Extend.h"
#include "ApplV_08_Manager.h"

namespace VENTOS {

class VehicleAdd : public BaseModule
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
	    int totalVehicles;
	    double lambda;

	    // methods
	    void Scenario1();
        void Scenario2();
        void Scenario3();
        void Scenario4();
        void Scenario5();
        void Scenario6();
        void Scenario7();
};

}

#endif

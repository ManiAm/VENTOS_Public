
#ifndef VEHICLEADD
#define VEHICLEADD

#include "BaseModule.h"
#include "TraCI_Extend.h"

#define SSTR( x ) dynamic_cast< std::ostringstream & >( (std::ostringstream() << std::dec << x ) ).str()

namespace VENTOS {

class AddVehicle : public BaseModule
{
	public:
		virtual ~AddVehicle();
		virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
		virtual void finish();
	    virtual void receiveSignal(cComponent *, simsignal_t, long);

	private:

        // NED variables
        cModule *nodePtr;   // pointer to the Node
        TraCI_Extend *TraCI;  // pointer to the TraCI module
        simsignal_t Signal_executeFirstTS;
        bool on;
        int mode;
	    int totalVehicles;
	    double lambda;
	    int plnSize;
	    double plnSpace;

	    // methods
        void Add();

	    void Scenario1();
        void Scenario2();
        void Scenario3();
        void Scenario4();
        void Scenario5();
        void Scenario6();
        void Scenario7();
        void Scenario8();
        void Scenario9();
};

}

#endif


#ifndef TRAFFICLIGHT
#define TRAFFICLIGHT

#include "Global_01_TraCI_Extend.h"
#include "ApplV_07_Manager.h"

namespace VENTOS {

class TrafficLight : public BaseModule
{
	public:
		virtual ~TrafficLight();
		virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
		virtual void finish();

	public:
	    void Execute();

	private:
        // NED variables
        cModule *nodePtr;   // pointer to the Node
        TraCI_Extend *TraCI;  // pointer to the TraCI module
};
}


#endif

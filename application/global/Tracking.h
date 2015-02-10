
#ifndef TRACKING
#define TRACKING

#include "BaseModule.h"
#include "TraCI_Extend.h"

namespace VENTOS {

class Tracking : public BaseModule
{
	public:
		virtual ~Tracking();
		virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
		virtual void finish();
        virtual void receiveSignal(cComponent *, simsignal_t, long);

	private:

        // NED variables
        cModule *nodePtr;   // pointer to the Node
        TraCI_Extend *TraCI;  // pointer to the TraCI module
        simsignal_t Signal_executeFirstTS;

        // NED variables (GUI)
        bool on;
        double zoom;
        double initialWindowsOffset;
        double trackingInterval;
        int mode;
        string trackingV;
        string trackingLane;
        double windowsOffset;

        // class variables
        cMessage* updataGUI;

        // methods
        void Start();
        void TrackingGUI();
};

}

#endif

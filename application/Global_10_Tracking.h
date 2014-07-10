
#ifndef TRACKING
#define TRACKING

#include <omnetpp.h>
#include "mobility/traci/TraCIScenarioManagerLaunchd.h"
#include "mobility/traci/TraCIMobility.h"
#include "mobility/traci/TraCIConstants.h"
#include "Global_01_TraCI_Extend.h"
#include "ApplV_06_Manager.h"
#include <sstream>
#include <iostream>
#include <fstream>

using namespace rapidxml;
using namespace std;


class Tracking : public cSimpleModule
{
	public:
		virtual ~Tracking();
		virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
		virtual void finish();

	public:
		void Start();

	private:

        // NED variables
        cModule *nodePtr;   // pointer to the Node
        TraCI_Extend *TraCI;  // pointer to the TraCI module

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
        void TrackingGUI();
};


#endif

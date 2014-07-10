
#ifndef TraCI_APP
#define TraCI_APP

#include <omnetpp.h>
#include "mobility/traci/TraCIScenarioManagerLaunchd.h"
#include "mobility/traci/TraCIMobility.h"
#include "mobility/traci/TraCIConstants.h"

#include "Global_01_TraCI_Extend.h"
#include "Global_03_Statistics.h"
#include "Global_04_VehicleAdd.h"
#include "Global_05_RSUAdd.h"
#include "Global_06_Warmup.h"
#include "Global_07_SpeedProfile.h"
#include "Global_09_TrafficLight.h"

#include <sstream>
#include <iostream>
#include <fstream>


class TraCI_App : public TraCI_Extend
{
	public:
		virtual ~TraCI_App();
		virtual void initialize(int stage);
	    virtual int numInitStages() const
	    {
	        return 3;
	    }
        virtual void handleSelfMsg(cMessage *msg);
		virtual void finish();

        virtual void init_traci();
        virtual void executeOneTimestep();

	private:
        void AddAdversaryModule();
        void AddRSUModules();
        void TrackingGUI();

	private:

        // NED variables
        cModule *nodePtr;   // pointer to the Node
        Warmup *WarmupPtr;
        SpeedProfile *SpeedProfilePtr;
        TrafficLight *tlPtr;
        Statistics *StatPtr;

        // NED variable
        double terminate;

        // NED variables (GUI)
        bool tracking;
        double zoom;
        double initialWindowsOffset;
        double trackingInterval;
        int trackingMode;
        string trackingV;
        string trackingLane;
        double windowsOffset;

	    // class variables
        cMessage* updataGUI;
};


#endif

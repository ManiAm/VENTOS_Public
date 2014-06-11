
#ifndef TraCI_APP
#define TraCI_APP

#include <omnetpp.h>
#include "mobility/traci/TraCIScenarioManagerLaunchd.h"
#include "mobility/traci/TraCIMobility.h"
#include "mobility/traci/TraCIConstants.h"

#include "Global_01_TraCI_Extend.h"
#include "Global_03_Statistics.h"
#include "Global_04_VehicleAdd.h"
#include "Global_05_Warmup.h"
#include "Global_06_SpeedProfile.h"
#include "Global_08_TrafficLight.h"

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

	private:

        // NED variables
        cModule *nodePtr;   // pointer to the Node
        VehicleAdd *AddVehiclePtr;
        SpeedProfile *SpeedProfilePtr;
        Warmup *WarmupPtr;
        TrafficLight *tlPtr;
        Statistics *StatPtr;

        // NED variable
        double terminate;

        // NED variables (GUI)
        bool tracking;
        string trackingV;
        double trackingInterval;

	    // class variables
        cMessage* updataGUI;
};


#endif

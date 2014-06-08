
#ifndef TraCI_APP
#define TraCI_APP

#include <omnetpp.h>
#include "mobility/traci/TraCIScenarioManagerLaunchd.h"
#include "mobility/traci/TraCIMobility.h"
#include "mobility/traci/TraCIConstants.h"

#include "Global_01_TraCI_Extend.h"
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

        // NED variables
        cModule *nodePtr;   // pointer to the Node
        VehicleAdd *AddVehiclePtr;
        SpeedProfile *SpeedProfilePtr;
        Warmup *WarmupPtr;
        TrafficLight *tlPtr;

        // NEW variables
        bool collectVehiclesData;
        bool collectInductionLoopData;

        // NED variables (GUI tracking)
        bool tracking;
        string trackingV;
        double trackingInterval;

	    // class variables
        FILE *f1;
	    int index;
	    FILE *f2;
	    double terminate;
        cMessage* updataGUI;
        vector<LoopDetector *> Vec_loopDetectors;

	    // methods
        void AddAdversaryModule();
        void AddRSUModules();
        list<Coord> getCirclePoints(RSUEntry*, double);
        void vehiclesData();
        void writeToFile_PerVehicle(string, string);
        void inductionLoops();
        void writeToFile_InductionLoop();
        int findInVector(vector<LoopDetector *> , const char *, const char *);
};


#endif

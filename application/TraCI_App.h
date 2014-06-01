
#ifndef TraCI_APP
#define TraCI_APP

#include <omnetpp.h>
#include "mobility/traci/TraCIScenarioManagerLaunchd.h"
#include "mobility/traci/TraCIMobility.h"
#include "mobility/traci/TraCIConstants.h"
#include "ExtraClasses.h"
#include "TraCI_Extend.h"
#include "VehicleAdd.h"
#include "SpeedProfile.h"
#include "Warmup.h"
#include "TrafficLight.h"


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
        std::string trackingV;
        double trackingInterval;

	    // class variables
	    FILE *f1;
	    int index;
	    FILE *f2;
	    double terminate;
        cMessage* updataGUI;
        std::vector<LoopDetector *> Vec_loopDetectors;

	    // methods
        void AddAdversaryModule();
        void AddRSUModules();
        void vehiclesData();
        void writeToFile_PerVehicle(std::string, std::string);
        void inductionLoops();
        void writeToFile_InductionLoop();
        int findInVector(std::vector<LoopDetector *> , const char *, const char *);
};


#endif

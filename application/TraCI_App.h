
#ifndef TraCI_APP
#define TraCI_APP

#include <omnetpp.h>
#include "mobility/traci/TraCIScenarioManagerLaunchd.h"
#include "mobility/traci/TraCIMobility.h"
#include "mobility/traci/TraCIConstants.h"
#include "TraCI_Extend.h"
#include "AddVehicle.h"
#include "SpeedProfile.h"
#include "Warmup.h"


class TraCI_App : public TraCI_Extend
{
	public:
		virtual ~TraCI_App();
		virtual void initialize(int stage);
        virtual void handleSelfMsg(cMessage *msg);
		virtual void finish();

        virtual void init_traci();
        virtual void executeOneTimestep();

	private:

        // NED variables
        cModule *nodePtr;   // pointer to the Node
        AddVehicle *AddVehiclePtr;
        SpeedProfile *SpeedProfilePtr;
        Warmup *WarmupPtr;

	    // class variables
	    FILE *f1;
	    int index;
	    FILE *f2;
	    double terminate;

	    // methods
        void writeToFile();
        void writeToFilePerVehicle(std::string, std::string);
        void inductionLoops();
};


#endif


#ifndef TraCI_APP
#define TraCI_APP

#include <omnetpp.h>
#include "mobility/traci/TraCIScenarioManagerLaunchd.h"
#include "mobility/traci/TraCIMobility.h"
#include "mobility/traci/TraCIConstants.h"
#include "TraCI_Extend.h"


class TraCI_App : public TraCI_Extend
{
	public:
		virtual ~TraCI_App();
		virtual void initialize(int stage);
        virtual void handleSelfMsg(cMessage *msg);

        virtual void init_traci();
        virtual void executeOneTimestep();
		virtual void finish();

	private:
        cModule *nodePtr;   // pointer to the Node

	    FILE *f1;
	    int index;

        int platoonSize;
        int platoonNumber;
	    int totalVehicles;

        double warmUpT; // the time that warm-up phase finishes
        bool IsWarmUpFinished;
        cMessage* warmupFinish;

	    int trajectoryMode;
	    std::string trajectory;
	    double terminate;

	    double old_speed;
	    double old_time;

        FILE *f2;
	    bool endOfFile;

	    void add_vehicle();

        void writeToFile();
        void writeToFilePerVehicle(std::string, std::string);

        bool warmUpFinished();
	    void Trajectory();
	    void AccelDecel(double, double, double);
	    void AccelDecelZikZak(double, double, double);
        void AccelDecelPeriodic(double, double, double, double);
	    void ExTrajectory(double);
};


#endif

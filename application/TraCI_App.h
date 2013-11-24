
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

        virtual void init_traci();
        virtual void executeOneTimestep();
		virtual void finish();

	private:
        cModule *nodePtr;   // pointer to the Node

	    FILE *f1;
	    int index;

	    int trajectoryMode;
	    std::string trajectory;
	    double terminate;

	    bool reached;
	    double old_speed;

        FILE *f2;
        FILE *f3;
	    bool endOfFile;

        void writeToFile();
        void writeToFilePerVehicle(std::string, std::string);

	    void Trajectory();
	    void AccelDecel(double);
	    void AccelDecelZikZak(double, double);
        void AccelDecelPeriodic(double, double, double);
	    void ExTrajectory();
	    void StabilityTest();
};


#endif

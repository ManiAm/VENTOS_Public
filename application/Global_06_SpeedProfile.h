
#ifndef SPEEDPROFILE
#define SPEEDPROFILE

#include <omnetpp.h>
#include "mobility/traci/TraCIMobility.h"
#include "mobility/traci/TraCIConstants.h"
#include "Global_01_TraCI_Extend.h"

#include <sstream>
#include <iostream>
#include <fstream>


class SpeedProfile : public cSimpleModule
{
	public:
		virtual ~SpeedProfile();
		virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
		virtual void finish();

	public:
        void Change();

	private:

        // NED variables
        cModule *nodePtr;   // pointer to the Node
        TraCI_Extend *TraCI;
        bool on;
	    int mode;
	    std::string laneId;
	    double minSpeed;
        double normalSpeed;
        double maxSpeed;
        double switchTime;

	    // class variables
        std::string profileVehicle;
        std::string lastProfileVehicle;
	    double old_speed;
	    double old_time;
	    double startTime;  // the time that speed profiling starts
        FILE *f2;
        bool endOfFile;

	    // method
	    void AccelDecel(double, double, double);
	    void AccelDecelZikZak(double, double, double);
        void AccelDecelPeriodic(double, double, double, double);
	    void ExTrajectory(double);
};


#endif

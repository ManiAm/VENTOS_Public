
#ifndef RSUADD
#define RSUADD

#include <omnetpp.h>
#include "mobility/traci/TraCIScenarioManagerLaunchd.h"
#include "mobility/traci/TraCIMobility.h"
#include "mobility/traci/TraCIConstants.h"
#include "Global_01_TraCI_Extend.h"
#include "ApplV_07_Manager.h"
#include <sstream>
#include <iostream>
#include <fstream>

using namespace rapidxml;
using namespace std;


class RSUAdd : public cSimpleModule
{
	public:
		virtual ~RSUAdd();
		virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
		virtual void finish();

        void Add();  // should be public!
        Coord *commandGetRSUsCoord(unsigned int);  // should be public!

	private:
        deque<RSUEntry*> commandReadRSUsCoord(string);
        void commandAddCirclePoly(string, string, const TraCIColor& color, Coord*, double);
        void Scenario1();

	private:
        // NED variables
        cModule *nodePtr;   // pointer to the Node
        TraCI_Extend *TraCI;  // pointer to the TraCI module
        bool on;
        int mode;
        boost::filesystem::path SUMOfullDirectory;
        deque<RSUEntry*> RSUs;
};


#endif

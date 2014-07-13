
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

	public:
        void Add();

	private:

        // NED variables
        cModule *nodePtr;   // pointer to the Node
        TraCI_Extend *TraCI;  // pointer to the TraCI module
        boost::filesystem::path SUMOfullDirectory;
        bool on;
        int mode;

        // methods
        void Scenario1();
};


#endif

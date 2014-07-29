
#ifndef RSUADD
#define RSUADD

#include "Global_01_TraCI_Extend.h"
#include "ApplV_07_Manager.h"


class RSUAdd : public BaseModule
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

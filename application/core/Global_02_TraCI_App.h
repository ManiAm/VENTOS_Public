
#ifndef TraCI_APP
#define TraCI_APP

#include "Global_01_TraCI_Extend.h"
#include "Global_03_Tracking.h"
#include "Global_04_Statistics.h"
#include "Global_06_VehicleAdd.h"
#include "Global_07_RSUAdd.h"
#include "Global_08_Warmup.h"
#include "Global_09_SpeedProfile.h"

namespace VENTOS {

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
        void TrackingGUI();

	private:

        // NED variables
        cModule *nodePtr;   // pointer to the Node
        Warmup *WarmupPtr;
        SpeedProfile *SpeedProfilePtr;
        Statistics *StatPtr;

        // NED variable
        double terminate;
};
}

#endif


#ifndef TraCI_APP
#define TraCI_APP

#include "TraCI_Extend.h"
#include "Global_02_Tracking.h"
#include "Global_03_AddVehicle.h"
#include "Global_04_AddRSU.h"
#include "Global_05_AddAdversary.h"
#include "Global_06_Statistics.h"
#include "Global_07_VehicleWarmup.h"
#include "Global_08_VehicleSpeedProfile.h"
#include "Global_10_TrafficLightControl.h"

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

        virtual void addModule(std::string nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id = "", double speed = -1, double angle = -1);

    private:
        void TrackingGUI();

    private:
        // NED variables
        cModule *nodePtr;   // pointer to the Node
        Warmup *WarmupPtr;
        SpeedProfile *SpeedProfilePtr;
        Statistics *StatPtr;
        TrafficLightControl *TrafficLightControlPtr;

        // NED variable
        double terminate;

        // NED (bicycles)
        string bikeModuleType;
        string bikeModuleName;
        string bikeModuleDisplayString;

        // NED (pedestrians)
        string pedModuleType;
        string pedModuleName;
        string pedModuleDisplayString;
};

}

#endif

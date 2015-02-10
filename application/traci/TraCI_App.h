
#ifndef TraCI_APP
#define TraCI_APP

#include "TraCI_Extend.h"

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

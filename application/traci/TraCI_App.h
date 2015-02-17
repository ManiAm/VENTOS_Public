
#ifndef TraCI_APP
#define TraCI_APP

#include "TraCI_Extend.h"
#include "Router.h"

namespace VENTOS {

class VehicleData
{
  public:
    int index;
    double time;

    char vehicleName[20];
    char vehicleType[20];

    char lane[20];
    double pos;

    double speed;
    double accel;
    char CFMode[30];

    double timeGapSetting;
    double spaceGap;
    double timeGap;

    VehicleData(int i, double d1,
                 const char *str1, const char *str2,
                 const char *str3, double d2,
                 double d3, double d4, const char *str4,
                 double d3a, double d5, double d6)
    {
        this->index = i;
        this->time = d1;

        strcpy(this->vehicleName, str1);
        strcpy(this->vehicleType, str2);

        strcpy(this->lane, str3);
        this->pos = d2;

        this->speed = d3;
        this->accel = d4;
        strcpy(this->CFMode, str4);

        this->timeGapSetting = d3a;
        this->spaceGap = d5;
        this->timeGap = d6;
    }
};


class LoopDetector
{
  public:
    char detectorName[20];
    char vehicleName[20];
    double entryTime;
    double leaveTime;
    double entrySpeed;
    double leaveSpeed;

    LoopDetector( const char *str1, const char *str2, double entryT, double leaveT, double entryS, double leaveS )
    {
        strcpy(this->detectorName, str1);
        strcpy(this->vehicleName, str2);

        this->entryTime = entryT;
        this->leaveTime = leaveT;

        this->entrySpeed = entryS;
        this->leaveSpeed = leaveS;
    }
};

class Router;   //Forward-declaration so TraCI_App may hold a Router*

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

    private:
        // NED variables
        cModule *nodePtr;   // pointer to the Node
        Router* router;
        double terminate;
        bool collectVehiclesData;
        bool useDetailedFilenames;
        bool collectInductionLoopData;

        std::set<std::string> subscribedPedestrians; /**< all pedestrians we have already subscribed to */

        // NED (bicycles)
        string bikeModuleType;
        string bikeModuleName;
        string bikeModuleDisplayString;

        // NED (pedestrians)
        string pedModuleType;
        string pedModuleName;
        string pedModuleDisplayString;

        vector<VehicleData *> Vec_vehiclesData;
        vector<LoopDetector *> Vec_loopDetectors;

        int index;

    private:
        virtual void init_traci();
        virtual void executeOneTimestep();
        void addPedestriansToOMNET();
        virtual void addModule(std::string nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id = "", double speed = -1, double angle = -1);

        void vehiclesData();
        void saveVehicleData(string);
        void vehiclesDataToFile();

        void inductionLoops();
        void inductionLoopToFile();

        int findInVector(vector<LoopDetector *> , const char *, const char *);
};

}

#endif

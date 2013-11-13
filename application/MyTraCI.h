
#ifndef MyTraCI_H
#define MyTraCI_H

#include <omnetpp.h>
#include "mobility/traci/TraCIScenarioManagerLaunchd.h"
#include "mobility/traci/TraCIMobility.h"
#include "mobility/traci/TraCIConstants.h"


class MyTraCI : public TraCIScenarioManagerLaunchd
{
	public:
		virtual ~MyTraCI();
		virtual void initialize(int stage);
        virtual void init_traci();
        virtual void executeOneTimestep();
		virtual void finish();

		// we add these getters to the veins
        uint32_t commandGetNoVehicles();
        std::list<std::string> commandGetVehicleList();
        std::list<std::string> commandGetLaneList();
        std::list<std::string> commandGetVehicleLaneList(std::string);
        double commandGetVehicleSpeed(std::string);
        double commandGetVehicleAccel(std::string);
        std::string commandGetVehicleType(std::string);
        double commandGetVehicleLength(std::string);
        double commandGetVehicleMaxDecel(std::string);
        std::list<std::string> commandGetVehiclesOnLane(std::string);
        Coord commandGetVehiclePos(std::string);

        // we add these setters to veins
        void commandSetPreceding(std::string, std::string);
        void commandSetPlatoonLeader(std::string, std::string);
        void commandSetModeSwitch(std::string, bool);

	private:
	    FILE *f1;
	    static int index;

	    int trajectoryMode;
	    std::string trajectory;

        FILE *f2;
	    static int fileLine;
	    static bool endOfFile;

	    void AccelDecel();
	    void ExTrajectory();
	    void writeToFile();
        void writeToFilePerVehicle(std::string, std::string);

        uint32_t genericGetInt32(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
        Coord genericGetCoordv2(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
};


#endif

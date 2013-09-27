
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

		// added traci command
        uint32_t commandGetNoVehicles();
        std::list<std::string> commandGetVehicleList();
        double commandGetVehicleSpeed(std::string);
        std::string commandGetVehicleType(std::string);
        double commandGetVehicleLength(std::string);
        double commandGetVehicleMaxDecel(std::string);
        std::list<std::string> commandGetVehiclesOnLane(std::string);
        Coord commandGetVehiclePos(std::string);

	private:
	    FILE *f1;

	    void AccelDecelManual();
        void writeToFile(std::string, std::string);

        uint32_t genericGetInt32(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
        Coord genericGetCoordv2(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
};


#endif

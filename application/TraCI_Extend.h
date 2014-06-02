
#ifndef TraCIEXTEND_H
#define TraCIEXTEND_H

#include <omnetpp.h>
#include "mobility/traci/TraCIMobility.h"
#include "mobility/traci/TraCIConstants.h"
#include "mobility/traci/TraCIColor.h"
#include "ExtraClasses.h"
#include <deque>

#include <boost/tokenizer.hpp>
using namespace boost;

#include "rapidxml-1.13/rapidxml.hpp"
#include "rapidxml-1.13/rapidxml_utils.hpp"
#include "rapidxml-1.13/rapidxml_print.hpp"
using namespace rapidxml;


class TraCI_Extend : public TraCIScenarioManager
{
	public:
		virtual ~TraCI_Extend();
		virtual void initialize(int stage);
        virtual void handleSelfMsg(cMessage *msg);

        virtual void init_traci();
		virtual void finish();

        // CMD_GET_VEHICLE_VARIABLE
        uint32_t commandGetNoVehicles();
        std::list<std::string> commandGetVehicleList();
        double commandGetVehicleSpeed(std::string);
        double commandGetVehicleAccel(std::string);
        std::string commandGetVehicleType(std::string);
        double commandGetVehicleLength(std::string);
        double commandGetVehicleMinGap(std::string);
        double commandGetVehicleMaxDecel(std::string);
        Coord commandGetVehiclePos(std::string);
        uint32_t commandGetLaneIndex(std::string);
        std::vector<std::string> commandGetLeading(std::string, double);
        std::string commandGetLeading_old(std::string);
        uint8_t * commandGetVehicleColor(std::string);

        // CMD_GET_INDUCTIONLOOP_VARIABLE
        std::list<std::string> commandGetLoopDetectorList();
        uint32_t commandGetLoopDetectorCount(std::string);
        double commandGetLoopDetectorSpeed(std::string);
        std::list<std::string> commandGetLoopDetectorVehicleList(std::string);
        std::vector<std::string> commandGetLoopDetectorVehicleData(std::string);

        // CMD_GET_VEHICLETYPE_VARIABLE
        double commandGetVehicleLength_Type(std::string);

        // CMD_GET_LANE_VARIABLE
        std::list<std::string> commandGetLaneList();
        std::list<std::string> commandGetVehicleLaneList(std::string);
        std::list<std::string> commandGetVehiclesOnLane(std::string);

        // RSU
        std::deque<RSUEntry*> commandReadRSUsCoord(std::string);
        Coord* commandGetRSUsCoord(unsigned int);

        // CMD_GET_SIM_VARIABLE
        double* commandGetNetworkBoundary();

        // control-related commands
        void commandTerminate();
        void commandSendFile(std::string);

        // subscription commands
        void commandSubscribeSimulation();
        void commandSubscribeVehicle();

        // CMD_SET_VEHICLE_VARIABLE
        void commandSetMaxAccel(std::string, double);
        void commandSetMaxDecel(std::string, double);
        void commandSetTg(std::string, double);
        void commandAddVehicleN(std::string, std::string, std::string, int32_t, double, double, uint8_t);
        void commandSetCFParameters(std::string, std::string);      // new defined command
        void commandSetDebug(std::string, bool);                    // new defined command
        void commandSetModeSwitch(std::string, bool);              // new defined command
        void commandSetVehicleColor(std::string nodeId, TraCIColor& color);
        void commandRemoveVehicle(std::string, uint8_t);
        void commandStopNodeExtended(std::string, std::string, double, uint8_t, double, uint8_t);
        void commandSetvClass(std::string, std::string);
        void commandChangeLane(std::string, uint8_t, double);
        void commandSetErrorGap(std::string, double);             // new defined command
        void commandSetErrorRelSpeed(std::string, double);       // new defined command

        // CMD_SET_GUI_VARIABLE
        void commandSetGUIZoom(double);
        void commandSetGUITrack(std::string);
        void commandSetGUIOffset(double, double);

        // Polygon
        void commandAddCirclePoly(std::string, std::string, TraCIColor, Coord, double);

	protected:
        int seed; /**< seed value to set in launch configuration, if missing (-1: current run number) */
        std::string VENTOSfullDirectory;
        std::string SUMODirectory;
        std::string SUMOfullDirectory;
        std::deque<RSUEntry*> RSUs;

	private:
        // generic methods for getters
        uint32_t genericGetInt32(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
        Coord genericGetCoordv2(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
        uint8_t* genericGetArrayUnsignedInt(uint8_t, std::string, uint8_t, uint8_t);

        std::string createLaunch();
};


#endif

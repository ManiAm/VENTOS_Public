
#ifndef TraCIEXTEND_H
#define TraCIEXTEND_H

#include <omnetpp.h>
#include "modules/mobility/traci/TraCIMobility.h"
#include "modules/mobility/traci/TraCIConstants.h"
#include "modules/mobility/traci/TraCIColor.h"
#include "Appl.h"
#include <deque>

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>
using namespace rapidxml;

// un-defining ev!
// why? http://stackoverflow.com/questions/24103469/cant-include-the-boost-filesystem-header
#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

#include <Eigen/Dense>
using namespace Eigen;

// adding this after including Eigen header
// why? http://stackoverflow.com/questions/5327325/conflict-between-boost-opencv-and-eigen-libraries
using namespace boost::filesystem;

#include <boost/tokenizer.hpp>
using namespace boost;

using namespace std;
using namespace Veins;


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
        list<string> commandGetVehicleList();
        double commandGetVehicleSpeed(string);
        double commandGetVehicleAccel(string);
        string commandGetVehicleType(string);
        double commandGetVehicleLength(string);
        double commandGetVehicleMinGap(string);
        double commandGetVehicleMaxDecel(string);
        Coord commandGetVehiclePos(string);
        uint32_t commandGetLaneIndex(string);
        vector<string> commandGetLeading(string, double);
        string commandGetLeading_old(string);
        uint8_t* commandGetVehicleColor(string);

        // CMD_GET_VEHICLETYPE_VARIABLE
        double commandGetVehicleLength_Type(string);

        // CMD_GET_ROUTE_VARIABLE
        list<string> commandGetRouteIds();

        // CMD_GET_INDUCTIONLOOP_VARIABLE
        list<string> commandGetLoopDetectorList();
        uint32_t commandGetLoopDetectorCount(string);
        double commandGetLoopDetectorSpeed(string);
        list<string> commandGetLoopDetectorVehicleList(string);
        vector<string> commandGetLoopDetectorVehicleData(string);

        // CMD_GET_LANE_VARIABLE
        list<string> commandGetLaneList();
        list<string> commandGetVehicleLaneList(string);
        list<string> commandGetVehiclesOnLane(string);

        // RSU
        deque<RSUEntry*> commandReadRSUsCoord(string);
        Coord *commandGetRSUsCoord(unsigned int);

        // CMD_GET_SIM_VARIABLE
        double* commandGetNetworkBoundary();

        // control-related commands
        void commandTerminate();
        void commandSendFile(string);

        // subscription commands
        void commandSubscribeSimulation();
        void commandSubscribeVehicle();

        // CMD_SET_ROUTE_VARIABLE
        void commandAddRoute(string name, list<string> route);

        // CMD_SET_VEHICLE_VARIABLE
        void commandSetRouteFromList(string id, list<string> value);
        void commandSetSpeed(string nodeId, double);
        void commandSetMaxAccel(string, double);
        void commandSetMaxDecel(string, double);
        void commandSetTg(string, double);
        void commandSetLaneChangeMode(string, int32_t);
        void commandAddVehicleN(string, string, string, int32_t, double, double, uint8_t);
        void commandSetCFParameters(string, string);      // new defined command
        void commandSetDebug(string, bool);               // new defined command
        void commandSetModeSwitch(string, bool);         // new defined command
        void commandSetVehicleColor(string nodeId, TraCIColor& color);
        void commandRemoveVehicle(string, uint8_t);
        void commandStopNodeExtended(string, string, double, uint8_t, double, uint8_t);
        void commandSetvClass(string, string);
        void commandChangeLane(string, uint8_t, double);
        void commandSetErrorGap(string, double);             // new defined command
        void commandSetErrorRelSpeed(string, double);       // new defined command

        // CMD_SET_GUI_VARIABLE
        void commandSetGUIZoom(double);
        void commandSetGUITrack(string);
        void commandSetGUIOffset(double, double);

        // Polygon
        void commandAddCirclePoly(string, string, const TraCIColor& color, Coord*, double);

        //
        void commandSetEdgeGlobalTravelTime(string, int32_t, int32_t, double);

	protected:
        int seed; /**< seed value to set in launch configuration, if missing (-1: current run number) */
        boost::filesystem::path SUMOfullDirectory;
        deque<RSUEntry*> RSUs;

	private:
        // generic methods for getters
        Coord genericGetCoordv2(uint8_t commandId, string objectId, uint8_t variableId, uint8_t responseId);
        uint8_t* genericGetArrayUnsignedInt(uint8_t, string, uint8_t, uint8_t);

        string createLaunch();
};


#endif

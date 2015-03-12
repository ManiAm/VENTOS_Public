/****************************************************************************/
/// @file    TraCI_Extend.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    August 2013
///
/****************************************************************************/
// VENTOS, Vehicular Network Open Simulator; see http:?
// Copyright (C) 2013-2015
/****************************************************************************/
//
// This file is part of VENTOS.
// VENTOS is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef TraCIEXTEND_H
#define TraCIEXTEND_H

#include "modules/mobility/traci/TraCIScenarioManager.h"
#include "modules/mobility/traci/TraCICommandInterface.h"
#include "modules/mobility/traci/TraCIConstants.h"
#include "Appl.h"

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

namespace VENTOS {

class TraCI_Extend : public TraCIScenarioManager
{
	public:
		virtual ~TraCI_Extend();
		virtual void initialize(int stage);
        virtual void handleSelfMsg(cMessage *msg);
        virtual void init_traci();
		virtual void finish();

    protected:
        boost::filesystem::path VENTOS_FullPath;
        boost::filesystem::path SUMO_Path;
        boost::filesystem::path SUMO_FullPath;

    public:
        // CMD_GET_SIM_VARIABLE
        double* commandGetNetworkBoundary();
        uint32_t commandGetMinExpectedVehicles();
        uint32_t commandGetArrivedVehiclesCount();

        // control-related commands
        void commandTerminate();

        // CMD_GET_VEHICLE_VARIABLE
        list<string> commandGetVehicleList();
        uint32_t commandGetVehicleCount();
        double commandGetVehicleSpeed(string);
        Coord commandGetVehiclePos(string);
        string commandGetVehicleEdgeId(string);
        string commandGetVehicleLaneId(string);
        uint32_t commandGetVehicleLaneIndex(string);
        list<string> commandGetVehicleEdgeList(string nodeId);
        string commandGetVehicleTypeId(string);
        uint8_t* commandGetVehicleColor(string);
        double commandGetVehicleLanePosition(std::string nodeId);
        double commandGetVehicleLength(string);
        double commandGetVehicleMaxDecel(string);
        double commandGetVehicleTimeGap(string);
        double commandGetVehicleMinGap(string);
        string commandGetLeadingVehicle_old(string);
        vector<string> commandGetLeadingVehicle(string, double);
        double commandGetVehicleAccel(string);    // new defined command
        int commandGetVehicleCFMode(string);      // new defined command

        // CMD_GET_VEHICLETYPE_VARIABLE
        double commandGetVehicleTypeLength(string);
        int commandGetVehicleControllerType(string);      // new defined command
        int commandGetVehicleControllerNumber(string);    // new defined command

        // CMD_GET_ROUTE_VARIABLE
        list<string> commandGetRouteIds();

        // CMD_GET_LANE_VARIABLE
        list<string> commandGetLaneList();
        list<string> commandGetLaneVehicleList(string);
        double commandGetLaneLength(string);
        double commandGetLaneMeanSpeed(string);

        // CMD_GET_INDUCTIONLOOP_VARIABLE
        list<string> commandGetLoopDetectorList();
        uint32_t commandGetLoopDetectorCount(string);
        double commandGetLoopDetectorSpeed(string);
        list<string> commandGetLoopDetectorVehicleList(string);
        double commandGetLoopDetectorLastTime(string);
        vector<string> commandGetLoopDetectorVehicleData(string);

		// CMD_GET_TL_VARIABLE
		list<string> commandGetTLIDList();
		uint32_t commandGetCurrentPhaseDuration(string TLid);
		uint32_t commandGetCurrentPhase(string TLid);
		uint32_t commandGetNextSwitchTime(string TLid);
		string commandGetCurrentProgram(string TLid);
		string commandGetTLState(string TLid);

        // CMD_GET_GUI_VARIABLE
        Coord commandGetGUIOffset();
        vector<double> commandGetGUIBoundry();

        // CMD_GET_POLYGON_VARIABLE
        list<string> commandGetPolygonList();
        std::list<Coord> commandGetPolygonShape(string polyId);

        // CMD_GET_PERSON
        list<string> commandGetPedestrianList();
        uint32_t commandGetPedestrianCount();
        string commandGetPedestrianTypeId(string);
        Coord commandGetPedestrianPos(string);
        double commandGetPedestrianSpeed(string);
        string commandGetPedestrianRoadId(string);


        // ###########################################################
        // ###########################################################

        // CMD_SET_VEHICLE_VARIABLE
        void commandVehicleStop(string, string, double, uint8_t, double, uint8_t);
        void commandChangeVehicleLane(string, uint8_t, double);
        void commandChangeVehicleSpeed(string, double);
        void commandChangeVehicleColor(string, const TraCIColor&);
        void commandChangeVehicleRoute(string, list<string> value);
        int32_t commandCreatLaneChangeMode(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
        void commandSetLaneChangeMode(string, int32_t);
        void commandAddVehicle(string, string, string, int32_t, double, double, uint8_t);
        void commandSetVehicleParking(string);
        void commandSetVehicleClass(string, string);
        void commandSetVehicleMaxAccel(string, double);
        void commandSetVehicleMaxDecel(string, double);
        void commandSetVehicleTg(string, double);
        // set the controller's parameters for this vehicle
        void commandSetVehicleControllerParameters(string, string);  // new defined command
        // set an error value for the gap
        void commandSetVehicleErrorGap(string, double);           // new defined command
        // set an error value for relative speed
        void commandSetVehicleErrorRelSpeed(string, double);      // new defined command
        // should the controller degrade to ACC?
        void commandSetVehicleDegradeToACC(string, bool);         // new defined command
        // should the debug info be printed in the SUMO output console?
        void commandSetVehicleDebug(string, bool);                // new defined command

        // CMD_SET_VEHICLETYPE_VARIABLE
        void commandSetMaxSpeed(string, double);
        void commandSetVint(string, double);              // new defined command
        void commandSetComfAccel(string, double);         // new defined command
        void commandSetComfDecel(string, double);         // new defined command

        // CMD_SET_ROUTE_VARIABLE
        void commandAddRoute(string name, list<string> route);

        // CMD_SET_EDGE_VARIABLE
        void commandSetEdgeGlobalTravelTime(string, int32_t, int32_t, double);

        // CMD_SET_LANE_VARIABLE
        void commandSetLaneVmax(string, double);

        // CMD_SET_INDUCTIONLOOP_VARIABLE
        void commandAddLoopDetector();

		// CMD_SET_TL_VARIABLE
        void commandSetTLState(string TLid, string value);
        void commandSetTLPhaseIndex(string TLid, int value);
        void commandSetTLProgram(string TLid, string value);
		void commandSetTLPhaseDurationRemaining(string TLid, int value);
		void commandSetTLPhaseDuration(string TLid, int value);

        // CMD_SET_GUI_VARIABLE
        void commandSetGUIZoom(double);
        void commandSetGUITrack(string);
        void commandSetGUIOffset(double, double);

        // CMD_SET_POLYGON_VARIABLE
        void commandAddPolygon(std::string polyId, std::string polyType, const TraCIColor& color, bool filled, int32_t layer, const std::list<TraCICoord>& points);

	private:
        void sendLaunchFile();

        // generic methods for getters
        Coord genericGetCoordv2(uint8_t commandId, string objectId, uint8_t variableId, uint8_t responseId);
        vector<double> genericGetBoundingBox(uint8_t commandId, string objectId, uint8_t variableId, uint8_t responseId);
        uint8_t* genericGetArrayUnsignedInt(uint8_t, string, uint8_t, uint8_t);
};

}

#endif

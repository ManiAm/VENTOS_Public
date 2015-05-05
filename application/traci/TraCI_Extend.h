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

    private:
        void sendLaunchFile();

        // generic methods for getters
        Coord genericGetCoordv2(uint8_t commandId, string objectId, uint8_t variableId, uint8_t responseId);
        vector<double> genericGetBoundingBox(uint8_t commandId, string objectId, uint8_t variableId, uint8_t responseId);
        uint8_t* genericGetArrayUnsignedInt(uint8_t, string, uint8_t, uint8_t);

    protected:
        boost::filesystem::path VENTOS_FullPath;
        boost::filesystem::path SUMO_Path;
        boost::filesystem::path SUMO_FullPath;

    public:

        // ################################################################
        //                            simulation
        // ################################################################

        // CMD_GET_SIM_VARIABLE
        double* simulationGetNetBoundary();
        uint32_t simulationGetMinExpectedNumber();
        uint32_t simulationGetArrivedNumber();

        // control-related commands
        void simulationTerminate();

        // ################################################################
        //                            vehicle
        // ################################################################

        // CMD_GET_VEHICLE_VARIABLE
        list<string> vehicleGetIDList();
        uint32_t vehicleGetIDCount();
        double vehicleGetSpeed(string);
        uint8_t* vehicleGetStopState(string);
        Coord vehicleGetPosition(string);
        string vehicleGetEdgeID(string);
        string vehicleGetLaneID(string);
        uint32_t vehicleGetLaneIndex(string);
        double vehicleGetLanePosition(string);
        string vehicleGetTypeID(string);
        string vehicleGetRouteID(string);
        list<string> vehicleGetRoute(string);
        string vehicleGetBestLanes(string);
        uint8_t* vehicleGetColor(string);
        uint8_t* vehicleGetSignals(string);
        double vehicleGetLength(string);
        double vehicleGetMinGap(string);
        double vehicleGetMaxAccel(string);
        double vehicleGetMaxDecel(string);
        double vehicleGetTimeGap(string);
        string vehicleGetClass(string);
        vector<string> vehicleGetLeader(string, double);
        double vehicleGetCurrentAccel(string);       // new command
        int vehicleGetCarFollowingMode(string);      // new command [returns the current ACC/CACC car following mode]
        int vehicleGetTrafficLightAhead(string);     // new command [if the traffic light ahead is yellow or red]

        // CMD_SET_VEHICLE_VARIABLE
        void vehicleSetStop(string, string, double, uint8_t, double, uint8_t);  // adds or modifies a stop with the given parameters.
        void vehicleResume(string);
        void vehicleSetSpeed(string, double);
        int32_t vehicleBuildLaneChangeMode(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
        void vehicleSetLaneChangeMode(string, int32_t);
        void vehicleChangeLane(string, uint8_t, double);
        void vehicleSetRoute(string, list<string> value);
        void vehicleSetRouteID(string, string);
        void vehicleSetColor(string, const TraCIColor&);
        void vehicleSetClass(string, string);
        void vehicleSetMaxAccel(string, double);
        void vehicleSetMaxDecel(string, double);
        void vehicleSetTimeGap(string, double);
        void vehicleAdd(string, string, string, int32_t, double, double, uint8_t);
        void vehicleRemove(string, uint8_t);
        void vehicleSetControllerParameters(string, string);  // new command [set the controller's parameters for this vehicle]
        void vehicleSetErrorGap(string, double);              // new command [set an error value for the gap]
        void vehicleSetErrorRelSpeed(string, double);         // new command [set an error value for relative speed]
        void vehicleSetDowngradeToACC(string, bool);          // new command [should the controller degrade to ACC ?]
        void vehicleSetDebug(string, bool);                   // new command [should the debug info be printed in the SUMO output console ?]

        // ################################################################
        //                          vehicle type
        // ################################################################

        // CMD_GET_VEHICLETYPE_VARIABLE
        list<string> vehicleTypeGetIDList();
        uint32_t vehicleTypeGetIDCount();
        double vehicleTypeGetLength(string);
        double vehicleTypeGetMaxSpeed(string);
        int vehicleTypeGetControllerType(string);      // new command
        int vehicleTypeGetControllerNumber(string);    // new command

        // CMD_SET_VEHICLETYPE_VARIABLE
        void vehicleTypeSetMaxSpeed(string, double);
        void vehicleTypeSetVint(string, double);        // new command
        void vehicleTypeSetComfAccel(string, double);   // new command
        void vehicleTypeSetComfDecel(string, double);   // new command

        // ################################################################
        //                              route
        // ################################################################

        // CMD_GET_ROUTE_VARIABLE
        list<string> routeGetIDList();
        uint32_t routeGetIDCount();
        list<string> routeGetEdges(string);

        // CMD_SET_ROUTE_VARIABLE
        void routeAdd(string, list<string>);

        // ################################################################
        //                              edge
        // ################################################################

        // CMD_GET_EDGE_VARIABLE
        list<string> edgeGetIDList();
        uint32_t edgeGetIDCount();
        double edgeGetMeanTravelTime(string);
        uint32_t edgeGetLastStepVehicleNumber(string);
        list<string> edgeGetLastStepVehicleIDs(string);
        double edgeGetLastStepMeanVehicleSpeed(string);
        double edgeGetLastStepMeanVehicleLength(string);
        list<string> edgeGetLastStepPersonIDs(string);

        // CMD_SET_EDGE_VARIABLE
        void edgeSetGlobalTravelTime(string, int32_t, int32_t, double);

        // ################################################################
        //                              lane
        // ################################################################

        // CMD_GET_LANE_VARIABLE
        list<string> laneGetIDList();
        uint32_t laneGetIDCount();
        string laneGetEdgeID(string);
        double laneGetLength(string);
        double laneGetMaxSpeed(string);
        uint32_t laneGetLastStepVehicleNumber(string);
        list<string> laneGetLastStepVehicleIDs(string);
        double laneGetLastStepMeanVehicleSpeed(string);
        double laneGetLastStepMeanVehicleLength(string);

        // CMD_SET_LANE_VARIABLE
        void laneSetMaxSpeed(string, double);

        // ################################################################
        //                 loop detector (E1-Detectors)
        // ################################################################

        // CMD_GET_INDUCTIONLOOP_VARIABLE
        list<string> LDGetIDList();
        uint32_t LDGetIDCount(string);
        string LDGetLaneID(string);
        double LDGetPosition(string);
        uint32_t LDGetLastStepVehicleNumber(string);
        list<string> LDGetLastStepVehicleIDs(string);
        double LDGetLastStepMeanVehicleSpeed(string);
        double LDGetLastDetectionTime(string);
        vector<string> LDGetLastStepVehicleData(string);

        // ################################################################
        //                lane area detector (E2-Detectors)
        // ################################################################

        // CMD_GET_AREAL_DETECTOR_VARIABLE
        list<string> LADGetIDList();
        uint32_t LADGetIDCount(string);
        uint32_t LADGetLastStepVehicleNumber(string);
        list<string> LADGetLastStepVehicleIDs(string);
        double LADGetLastStepMeanVehicleSpeed(string);

        // ################################################################
        //                          traffic light
        // ################################################################

        // CMD_GET_TL_VARIABLE
		list<string> TLGetIDList();
		uint32_t TLGetIDCount();
        list<string> TLGetControlledLanes(string);
        map<int,string> TLGetControlledLinks(string);
        string TLGetProgram(string);
        uint32_t TLGetPhase(string);
        string TLGetState(string);
        uint32_t TLGetPhaseDuration(string);
		uint32_t TLGetNextSwitchTime(string);

        // CMD_SET_TL_VARIABLE
        void TLSetProgram(string, string);
        void TLSetPhaseIndex(string, int);
        void TLSetPhaseDuration(string, int);
        void TLSetState(string, string);

        // ################################################################
        //                               GUI
        // ################################################################

        // CMD_GET_GUI_VARIABLE
        Coord GUIGetOffset();
        vector<double> GUIGetBoundry();

        // CMD_SET_GUI_VARIABLE
        void GUISetZoom(double);
        void GUISetTrackVehicle(string);
        void GUISetOffset(double, double);

        // ################################################################
        //                               polygon
        // ################################################################

        // CMD_GET_POLYGON_VARIABLE
        list<string> polygonGetIDList();
        uint32_t polygonGetIDCount();
        list<Coord> polygonGetShape(string);

        // CMD_SET_POLYGON_VARIABLE
        void polygonAdd(string, string, const TraCIColor&, bool, int32_t, const list<TraCICoord>&);
        void polygonSetFilled(string, uint8_t);

        // ################################################################
        //                               person
        // ################################################################

        // CMD_GET_PERSON
        list<string> personGetIDList();
        uint32_t personGetIDCount();
        string personGetTypeID(string);
        Coord personGetPosition(string);
        string personGetEdgeID(string);
        double personGetEdgePosition(string);
        double personGetSpeed(string);
        string personGetNextEdge(string);
};

}

#endif

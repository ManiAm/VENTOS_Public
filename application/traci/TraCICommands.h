/****************************************************************************/
/// @file    TraCI_Commands.h
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

#ifndef TraCICOMMANDS_H
#define TraCICOMMANDS_H

#include "TraCIConnection.h"
#include "TraCIBuffer.h"
#include "Color.h"
#include "TraCICoord.h"

#include "Coord.h"
#include "ConstsVENTOS.h"

#include <chrono>
#include <ctime>
#include <ratio>

namespace VENTOS {

class bestLanesEntry
{
public:
    std::string laneId;
    double length;
    double occupation;
    int offset;
    int continuingDrive;
    std::vector<std::string> best;

    bestLanesEntry()
    {
        this->laneId = "";
        this->length = -1;
        this->occupation = -1;
        this->offset = -1;
        this->continuingDrive = -1;
        this->best.clear();
    }
};


class linkEntry
{
public:
    std::string consecutive1;
    std::string consecutive2;
    int priority;
    int opened;
    int approachingFoe;
    std::string state;
    std::string direction;
    double length;

    linkEntry()
    {
        this->consecutive1 = "";
        this->consecutive2 = "";
        this->priority = -1;
        this->opened = -1;
        this->approachingFoe = -1;
        this->state = "";
        this->direction = "";
        this->length = -1;
    }
};


typedef std::chrono::high_resolution_clock::time_point Htime_t;

// logging TraCI command exchange
class TraCIcommandEntry
{
public:
    double timeStamp;
    Htime_t sentAt;
    Htime_t completeAt;
    uint8_t commandGroupId;
    uint8_t commandId;

    TraCIcommandEntry(double ts, Htime_t st, Htime_t et, uint8_t cg, uint8_t c)
    {
        this->timeStamp = ts;
        this->sentAt = st;
        this->completeAt = et;
        this->commandGroupId = cg;
        this->commandId = c;
    }
};


class TraCI_Commands : public cSimpleModule
{
public:
    virtual ~TraCI_Commands();
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    // ################################################################
    //                           python interaction
    // ################################################################

    std::pair<uint32_t, std::string> getVersion();

    // ################################################################
    //                      simulation control
    // ################################################################

    void simulationTerminate();
    std::pair<TraCIBuffer, uint32_t> simulationTimeStep(uint32_t targetTime);

    // ################################################################
    //                          subscription
    // ################################################################

    // CMD_SUBSCRIBE_SIM_VARIABLE
    TraCIBuffer simulationSubscribe(uint32_t beginTime, uint32_t endTime, std::string objectId, std::vector<uint8_t> variables);
    // CMD_SUBSCRIBE_VEHICLE_VARIABLE
    TraCIBuffer vehicleSubscribe(uint32_t beginTime, uint32_t endTime, std::string objectId, std::vector<uint8_t> variables);

    // ################################################################
    //                            simulation
    // ################################################################

    // CMD_GET_SIM_VARIABLE
    uint32_t simulationGetLoadedVehiclesCount();
    std::list<std::string> simulationGetLoadedVehiclesIDList();
    double* simulationGetNetBoundary();
    uint32_t simulationGetMinExpectedNumber();
    uint32_t simulationGetArrivedNumber();

    // ################################################################
    //                            vehicle
    // ################################################################

    // CMD_GET_VEHICLE_VARIABLE
    std::list<std::string> vehicleGetIDList();
    uint32_t vehicleGetIDCount();
    double vehicleGetSpeed(std::string);
    uint8_t vehicleGetStopState(std::string);
    Coord vehicleGetPosition(std::string);
    std::string vehicleGetEdgeID(std::string);
    std::string vehicleGetLaneID(std::string);
    uint32_t vehicleGetLaneIndex(std::string);
    double vehicleGetLanePosition(std::string);
    std::string vehicleGetTypeID(std::string);
    std::string vehicleGetRouteID(std::string);
    std::list<std::string> vehicleGetRoute(std::string);
    uint32_t vehicleGetRouteIndex(std::string);
    std::map<int,bestLanesEntry> vehicleGetBestLanes(std::string);
    uint8_t* vehicleGetColor(std::string);
    uint32_t vehicleGetSignalStatus(std::string);
    double vehicleGetLength(std::string);
    double vehicleGetMinGap(std::string);
    double vehicleGetMaxAccel(std::string);
    double vehicleGetMaxDecel(std::string);
    double vehicleGetTimeGap(std::string);
    std::string vehicleGetClass(std::string);
    std::vector<std::string> vehicleGetLeader(std::string, double);
    double vehicleGetCurrentAccel(std::string);       // new command
    int vehicleGetCarFollowingMode(std::string);      // new command [returns the current ACC/CACC car following mode]
    std::string vehicleGetTLID(std::string);          // new command [returns the id of the TL ahead]          // todo: remove this command from here and SUMO
    char vehicleGetTLLinkStatus(std::string);         // new command [return the TL status ahead: g, G, Y, R]  // todo: remove this command once implemented by SUMO

    // CMD_SET_VEHICLE_VARIABLE
    void vehicleSetStop(std::string, std::string, double, uint8_t, double, uint8_t);  // adds or modifies a stop with the given parameters
    void vehicleResume(std::string);
    void vehicleSetSpeed(std::string, double);
    int32_t vehicleBuildLaneChangeMode(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
    void vehicleSetLaneChangeMode(std::string, int32_t);
    void vehicleChangeLane(std::string, uint8_t, double);
    void vehicleSetRoute(std::string, std::list<std::string> value);
    void vehicleSetRouteID(std::string, std::string);
    void vehicleSetColor(std::string, const RGB);
    void vehicleSetClass(std::string, std::string);
    void vehicleSetMaxAccel(std::string, double);
    void vehicleSetMaxDecel(std::string, double);
    void vehicleSetTimeGap(std::string, double);
    void vehicleAdd(std::string, std::string, std::string, int32_t, double, double, uint8_t);
    void vehicleRemove(std::string, uint8_t);
    void vehicleSetControllerParameters(std::string, std::string);  // new command [set the controller's parameters for this vehicle]
    void vehicleSetErrorGap(std::string, double);                   // new command [set an error value for the gap]
    void vehicleSetErrorRelSpeed(std::string, double);              // new command [set an error value for relative speed]
    void vehicleSetDowngradeToACC(std::string, bool);               // new command [should the controller degrade to ACC ?]
    void vehicleSetDebug(std::string, bool);                        // new command [should the debug info be printed in the SUMO output console ?]

    // ################################################################
    //                          vehicle type
    // ################################################################

    // CMD_GET_VEHICLETYPE_VARIABLE
    std::list<std::string> vehicleTypeGetIDList();
    uint32_t vehicleTypeGetIDCount();
    double vehicleTypeGetLength(std::string);
    double vehicleTypeGetMaxSpeed(std::string);
    int vehicleTypeGetControllerType(std::string);      // new command
    int vehicleTypeGetControllerNumber(std::string);    // new command

    // CMD_SET_VEHICLETYPE_VARIABLE
    void vehicleTypeSetMaxSpeed(std::string, double);
    void vehicleTypeSetVint(std::string, double);       // new command
    void vehicleTypeSetComfAccel(std::string, double);  // new command
    void vehicleTypeSetComfDecel(std::string, double);  // new command

    // ################################################################
    //                              route
    // ################################################################

    // CMD_GET_ROUTE_VARIABLE
    std::list<std::string> routeGetIDList();
    uint32_t routeGetIDCount();
    std::list<std::string> routeGetEdges(std::string);

    // CMD_SET_ROUTE_VARIABLE
    void routeAdd(std::string, std::list<std::string>);

    // ################################################################
    //                              edge
    // ################################################################

    // CMD_GET_EDGE_VARIABLE
    std::list<std::string> edgeGetIDList();
    uint32_t edgeGetIDCount();
    double edgeGetMeanTravelTime(std::string);
    uint32_t edgeGetLastStepVehicleNumber(std::string);
    std::list<std::string> edgeGetLastStepVehicleIDs(std::string);
    double edgeGetLastStepMeanVehicleSpeed(std::string);
    double edgeGetLastStepMeanVehicleLength(std::string);
    std::list<std::string> edgeGetLastStepPersonIDs(std::string);

    // CMD_SET_EDGE_VARIABLE
    void edgeSetGlobalTravelTime(std::string, int32_t, int32_t, double);

    // ################################################################
    //                              lane
    // ################################################################

    // CMD_GET_LANE_VARIABLE
    std::list<std::string> laneGetIDList();
    uint32_t laneGetIDCount();
    uint8_t laneGetLinkNumber(std::string);
    std::map<int,linkEntry> laneGetLinks(std::string);
    std::list<std::string> laneGetAllowedClasses(std::string);
    std::string laneGetEdgeID(std::string);
    double laneGetLength(std::string);
    double laneGetMaxSpeed(std::string);
    uint32_t laneGetLastStepVehicleNumber(std::string);
    std::list<std::string> laneGetLastStepVehicleIDs(std::string);
    double laneGetLastStepMeanVehicleSpeed(std::string);
    double laneGetLastStepMeanVehicleLength(std::string);

    // CMD_SET_LANE_VARIABLE
    void laneSetMaxSpeed(std::string, double);

    // ################################################################
    //                 loop detector (E1-Detectors)
    // ################################################################

    // CMD_GET_INDUCTIONLOOP_VARIABLE
    std::list<std::string> LDGetIDList();
    uint32_t LDGetIDCount(std::string);
    std::string LDGetLaneID(std::string);
    double LDGetPosition(std::string);
    uint32_t LDGetLastStepVehicleNumber(std::string);
    std::list<std::string> LDGetLastStepVehicleIDs(std::string);
    double LDGetLastStepMeanVehicleSpeed(std::string);
    double LDGetElapsedTimeLastDetection(std::string);
    std::vector<std::string> LDGetLastStepVehicleData(std::string);

    // ################################################################
    //                lane area detector (E2-Detectors)
    // ################################################################

    // CMD_GET_AREAL_DETECTOR_VARIABLE
    std::list<std::string> LADGetIDList();
    uint32_t LADGetIDCount(std::string);
    std::string LADGetLaneID(std::string);
    uint32_t LADGetLastStepVehicleNumber(std::string);
    std::list<std::string> LADGetLastStepVehicleIDs(std::string);
    double LADGetLastStepMeanVehicleSpeed(std::string);
    uint32_t LADGetLastStepVehicleHaltingNumber(std::string);
    double LADGetLastStepJamLengthInMeter(std::string);

    // ################################################################
    //                          traffic light
    // ################################################################

    // CMD_GET_TL_VARIABLE
    std::list<std::string> TLGetIDList();
    uint32_t TLGetIDCount();
    std::list<std::string> TLGetControlledLanes(std::string);
    std::map<int,std::vector<std::string>> TLGetControlledLinks(std::string);
    std::string TLGetProgram(std::string);
    uint32_t TLGetPhase(std::string);
    std::string TLGetState(std::string);
    uint32_t TLGetPhaseDuration(std::string);
    uint32_t TLGetNextSwitchTime(std::string);

    // CMD_SET_TL_VARIABLE
    void TLSetProgram(std::string, std::string);
    void TLSetPhaseIndex(std::string, int);
    void TLSetPhaseDuration(std::string, int);
    void TLSetState(std::string, std::string);

    // ################################################################
    //                             Junction
    // ################################################################

    // CMD_GET_JUNCTION_VARIABLE
    std::list<std::string> junctionGetIDList();
    uint32_t junctionGetIDCount();
    Coord junctionGetPosition(std::string);

    // ################################################################
    //                               GUI
    // ################################################################

    // CMD_GET_GUI_VARIABLE
    Coord GUIGetOffset(std::string);
    std::vector<double> GUIGetBoundry(std::string);

    // CMD_SET_GUI_VARIABLE
    void GUISetZoom(std::string, double);
    void GUISetTrackVehicle(std::string, std::string);
    void GUISetOffset(std::string, double, double);

    // ################################################################
    //                               polygon
    // ################################################################

    // CMD_GET_POLYGON_VARIABLE
    std::list<std::string> polygonGetIDList();
    uint32_t polygonGetIDCount();
    std::list<Coord> polygonGetShape(std::string);
    std::string polygonGetTypeID(std::string);

    // CMD_SET_POLYGON_VARIABLE
    void polygonAddTraCI(std::string, std::string, const RGB, bool, int32_t, const std::list<TraCICoord>&);
    void polygonAdd(std::string, std::string, const RGB, bool, int32_t, const std::list<Coord>&);
    void polygonSetFilled(std::string, uint8_t);

    // ################################################################
    //                               POI
    // ################################################################

    void addPoi(std::string, std::string, const RGB, int32_t, const Coord&);


    // ################################################################
    //                               person
    // ################################################################

    // CMD_GET_PERSON
    std::list<std::string> personGetIDList();
    uint32_t personGetIDCount();
    std::string personGetTypeID(std::string);
    Coord personGetPosition(std::string);
    std::string personGetEdgeID(std::string);
    double personGetEdgePosition(std::string);
    double personGetSpeed(std::string);
    std::string personGetNextEdge(std::string);


    // ################################################################
    //                      coordinate conversion
    // ################################################################

    double traci2omnetAngle(double angle) const;    // convert TraCI angle to OMNeT++ angle (in rad)
    double omnet2traciAngle(double angle) const;    // convert OMNeT++ angle (in rad) to TraCI angle

    Coord traci2omnet(TraCICoord coord) const;      // convert TraCI coordinates to OMNeT++ coordinates
    TraCICoord omnet2traci(Coord coord) const;      // convert OMNeT++ coordinates to TraCI coordinates


    // ################################################################
    //                       SUMO directory
    // ################################################################

    std::string getSUMOFullDir();
    std::string getSUMOConfigFullPath();

private:
    // ################################################################
    //                    generic methods for getters
    // ################################################################

    double genericGetDouble(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
    int32_t genericGetInt(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
    std::string genericGetString(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
    std::list<std::string> genericGetStringList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
    Coord genericGetCoord(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
    std::list<Coord> genericGetCoordList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);

    uint8_t genericGetUnsignedByte(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
    Coord genericGetCoordv2(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
    std::vector<double> genericGetBoundingBox(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
    uint8_t* genericGetArrayUnsignedInt(uint8_t, std::string, uint8_t, uint8_t);

    // method used internally to log TraCI exchange
    void updateTraCIlog(std::string, uint8_t, uint8_t);
    void TraCIexchangeToFile();

protected:
    // these variables are set by TraCIStart class
    TraCIConnection* connection = NULL;
    TraCICoord netbounds1;   /* network boundaries as reported by TraCI (x1, y1) */
    TraCICoord netbounds2;   /* network boundaries as reported by TraCI (x2, y2) */

private:
    bool logTraCIcommands;
    int margin;
    std::vector<TraCIcommandEntry> exchangedTraCIcommands;

    std::map<std::pair<uint8_t /*command group*/, uint8_t /*command*/>, std::string /*command str*/> TraCIcommandsMap = {

            {{0x00, 0xff}, "getVersion"},
            {{0x02, 0xff}, "simulationTimeStep"},
            {{0x7f, 0xff}, "simulationTerminate"},

            {{0xdb, 0xff}, "simulationSubscribe"},
            {{0xd4, 0xff}, "vehicleSubscribe"},

            {{0xab, 0x71}, "simulationGetLoadedVehiclesCount"},
            {{0xab, 0x72}, "simulationGetLoadedVehiclesIDList"},
            {{0xab, 0x7c}, "simulationGetNetBoundary"},
            {{0xab, 0x7d}, "simulationGetMinExpectedNumber"},
            {{0xab, 0x79}, "simulationGetArrivedNumber"},

            {{0xa4, 0x00}, "vehicleGetIDList"},
            {{0xa4, 0x01}, "vehicleGetIDCount"},
            {{0xa4, 0x40}, "vehicleGetSpeed"},
            {{0xa4, 0x5b}, "vehicleGetStopState"},
            {{0xa4, 0x42}, "vehicleGetPosition"},
            {{0xa4, 0x50}, "vehicleGetEdgeID"},
            {{0xa4, 0x51}, "vehicleGetLaneID"},
            {{0xa4, 0x52}, "vehicleGetLaneIndex"},
            {{0xa4, 0x56}, "vehicleGetLanePosition"},
            {{0xa4, 0x4f}, "vehicleGetTypeID"},
            {{0xa4, 0x53}, "vehicleGetRouteID"},
            {{0xa4, 0x54}, "vehicleGetRoute"},
            {{0xa4, 0x69}, "vehicleGetRouteIndex"},
            {{0xa4, 0xb2}, "vehicleGetBestLanes"},
            {{0xa4, 0x45}, "vehicleGetColor"},
            {{0xa4, 0x5b}, "vehicleGetSignalStatus"},
            {{0xa4, 0x44}, "vehicleGetLength"},
            {{0xa4, 0x4c}, "vehicleGetMinGap"},
            {{0xa4, 0x46}, "vehicleGetMaxAccel"},
            {{0xa4, 0x47}, "vehicleGetMaxDecel"},
            {{0xa4, 0x48}, "vehicleGetTimeGap"},
            {{0xa4, 0x49}, "vehicleGetClass"},
            {{0xa4, 0x68}, "vehicleGetLeader"},
            {{0xa4, 0x70}, "vehicleGetCurrentAccel"},
            {{0xa4, 0x71}, "vehicleGetCarFollowingMode"},
            {{0xa4, 0x72}, "vehicleGetTLID"},
            {{0xa4, 0x73}, "vehicleGetTLLinkStatus"},

            {{0xc4, 0x12}, "vehicleSetStop"},
            {{0xc4, 0x19}, "vehicleResume"},
            {{0xc4, 0x40}, "vehicleSetSpeed"},
            {{0xc4, 0xb6}, "vehicleSetLaneChangeMode"},
            {{0xc4, 0x13}, "vehicleChangeLane"},
            {{0xc4, 0x57}, "vehicleSetRoute"},
            {{0xc4, 0x53}, "vehicleSetRouteID"},
            {{0xc4, 0x45}, "vehicleSetColor"},
            {{0xc4, 0x49}, "vehicleSetClass"},
            {{0xc4, 0x46}, "vehicleSetMaxAccel"},
            {{0xc4, 0x47}, "vehicleSetMaxDecel"},
            {{0xc4, 0x48}, "vehicleSetTimeGap"},
            {{0xc4, 0x80}, "vehicleAdd"},
            {{0xc4, 0x81}, "vehicleRemove"},
            {{0xc4, 0x15}, "vehicleSetControllerParameters"},
            {{0xc4, 0x20}, "vehicleSetErrorGap"},
            {{0xc4, 0x21}, "vehicleSetErrorRelSpeed"},
            {{0xc4, 0x17}, "vehicleSetDowngradeToACC"},
            {{0xc4, 0x16}, "vehicleSetDebug"},

            {{0xa5, 0x00}, "vehicleTypeGetIDList"},
            {{0xa5, 0x01}, "vehicleTypeGetIDCount"},
            {{0xa5, 0x44}, "vehicleTypeGetLength"},
            {{0xa5, 0x41}, "vehicleTypeGetMaxSpeed"},
            {{0xa5, 0x02}, "vehicleTypeGetControllerType"},
            {{0xa5, 0x03}, "vehicleTypeGetControllerNumber"},

            {{0xc5, 0x41}, "vehicleTypeSetMaxSpeed"},
            {{0xc5, 0x22}, "vehicleTypeSetVint"},
            {{0xc5, 0x23}, "vehicleTypeSetComfAccel"},
            {{0xc5, 0x24}, "vehicleTypeSetComfDecel"},

            {{0xa6, 0x00}, "routeGetIDList"},
            {{0xa6, 0x01}, "routeGetIDCount"},
            {{0xa6, 0x54}, "routeGetEdges"},

            {{0xc6, 0x0e}, "routeAdd"},

            {{0xaa, 0x00}, "edgeGetIDList"},
            {{0xaa, 0x01}, "edgeGetIDCount"},
            {{0xaa, 0x5a}, "edgeGetMeanTravelTime"},
            {{0xaa, 0x10}, "edgeGetLastStepVehicleNumber"},
            {{0xaa, 0x12}, "edgeGetLastStepVehicleIDs"},
            {{0xaa, 0x11}, "edgeGetLastStepMeanVehicleSpeed"},
            {{0xaa, 0x15}, "edgeGetLastStepMeanVehicleLength"},
            {{0xaa, 0x1a}, "edgeGetLastStepPersonIDs"},

            {{0xca, 0x58}, "edgeSetGlobalTravelTime"},

            {{0xa3, 0x00}, "laneGetIDList"},
            {{0xa3, 0x01}, "laneGetIDCount"},
            {{0xa3, 0x30}, "laneGetLinkNumber"},
            {{0xa3, 0x33}, "laneGetLinks"},
            {{0xa3, 0x34}, "laneGetAllowedClasses"},
            {{0xa3, 0x31}, "laneGetEdgeID"},
            {{0xa3, 0x44}, "laneGetLength"},
            {{0xa3, 0x41}, "laneGetMaxSpeed"},
            {{0xa3, 0x10}, "laneGetLastStepVehicleNumber"},
            {{0xa3, 0x12}, "laneGetLastStepVehicleIDs"},
            {{0xa3, 0x11}, "laneGetLastStepMeanVehicleSpeed"},
            {{0xa3, 0x15}, "laneGetLastStepMeanVehicleLength"},

            {{0xc3, 0x41}, "laneSetMaxSpeed"},

            {{0xa0, 0x00}, "LDGetIDList"},
            {{0xa0, 0x01}, "LDGetIDCount"},
            {{0xa0, 0x51}, "LDGetLaneID"},
            {{0xa0, 0x42}, "LDGetPosition"},
            {{0xa0, 0x10}, "LDGetLastStepVehicleNumber"},
            {{0xa0, 0x12}, "LDGetLastStepVehicleIDs"},
            {{0xa0, 0x11}, "LDGetLastStepMeanVehicleSpeed"},
            {{0xa0, 0x16}, "LDGetElapsedTimeLastDetection"},
            {{0xa0, 0x17}, "LDGetLastStepVehicleData"},

            {{0xad, 0x00}, "LADGetIDList"},
            {{0xad, 0x01}, "LADGetIDCount"},
            {{0xad, 0x51}, "LADGetLaneID"},
            {{0xad, 0x10}, "LADGetLastStepVehicleNumber"},
            {{0xad, 0x12}, "LADGetLastStepVehicleIDs"},
            {{0xad, 0x11}, "LADGetLastStepMeanVehicleSpeed"},
            {{0xad, 0x14}, "LADGetLastStepVehicleHaltingNumber"},
            {{0xad, 0x19}, "LADGetLastStepJamLengthInMeter"},

            {{0xa2, 0x00}, "TLGetIDList"},
            {{0xa2, 0x01}, "TLGetIDCount"},
            {{0xa2, 0x26}, "TLGetControlledLanes"},
            {{0xa2, 0x27}, "TLGetControlledLinks"},
            {{0xa2, 0x29}, "TLGetProgram"},
            {{0xa2, 0x28}, "TLGetPhase"},
            {{0xa2, 0x20}, "TLGetState"},
            {{0xa2, 0x24}, "TLGetPhaseDuration"},
            {{0xa2, 0x2d}, "TLGetNextSwitchTime"},

            {{0xc2, 0x23}, "TLSetProgram"},
            {{0xc2, 0x22}, "TLSetPhaseIndex"},
            {{0xc2, 0x24}, "TLSetPhaseDuration"},
            {{0xc2, 0x20}, "TLSetState"},

            {{0xa9, 0x00}, "junctionGetIDList"},
            {{0xa9, 0x01}, "junctionGetIDCount"},
            {{0xa9, 0x42}, "junctionGetPosition"},

            {{0xac, 0xa1}, "GUIGetOffset"},
            {{0xac, 0xa3}, "GUIGetBoundry"},

            {{0xcc, 0xa0}, "GUISetZoom"},
            {{0xcc, 0xa1}, "GUISetOffset"},
            {{0xcc, 0xa6}, "GUISetTrackVehicle"},

            {{0xa8, 0x00}, "polygonGetIDList"},
            {{0xa8, 0x01}, "polygonGetIDCount"},
            {{0xa8, 0x4e}, "polygonGetShape"},
            {{0xa8, 0x4f}, "polygonGetTypeID"},

            {{0xc8, 0x80}, "polygonAdd"},
            {{0xc8, 0x55}, "polygonSetFilled"},

            {{0xc7, 0x80}, "addPoi"},

            {{0xae, 0x00}, "personGetIDList"},
            {{0xae, 0x01}, "personGetIDCount"},
            {{0xae, 0x4f}, "personGetTypeID"},
            {{0xae, 0x42}, "personGetPosition"},
            {{0xae, 0x50}, "personGetEdgeID"},
            {{0xae, 0x56}, "personGetEdgePosition"},
            {{0xae, 0x40}, "personGetSpeed"},
            {{0xae, 0xc1}, "personGetNextEdge"}
    };
};

}

#endif

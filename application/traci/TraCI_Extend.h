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

#include "TraCI_Base.h"

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


class TraCI_Extend : public TraCI_Base
{
public:
    virtual ~TraCI_Extend();
    virtual void initialize(int stage);
    virtual void handleSelfMsg(cMessage *msg);
    virtual void finish();

    // ################################################################
    //                            simulation
    // ################################################################

    std::pair<uint32_t, std::string> getVersion();

    // CMD_GET_SIM_VARIABLE
    uint32_t simulationGetLoadedVehiclesCount();
    std::list<std::string> simulationGetLoadedVehiclesIDList();
    double* simulationGetNetBoundary();
    uint32_t simulationGetMinExpectedNumber();
    uint32_t simulationGetArrivedNumber();

    // control-related commands
    void simulationTerminate();

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
    void vehicleSetColor(std::string, const TraCIColor&);
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
    Coord GUIGetOffset();
    std::vector<double> GUIGetBoundry();

    // CMD_SET_GUI_VARIABLE
    void GUISetZoom(double);
    void GUISetTrackVehicle(std::string);
    void GUISetOffset(double, double);

    // ################################################################
    //                               polygon
    // ################################################################

    // CMD_GET_POLYGON_VARIABLE
    std::list<std::string> polygonGetIDList();
    uint32_t polygonGetIDCount();
    std::list<Coord> polygonGetShape(std::string);

    // CMD_SET_POLYGON_VARIABLE
    void polygonAddTraCI(std::string, std::string, const TraCIColor&, bool, int32_t, const std::list<TraCICoord>&);
    void polygonAdd(std::string, std::string, const TraCIColor&, bool, int32_t, const std::list<Coord>&);
    void polygonSetFilled(std::string, uint8_t);

    // ################################################################
    //                               polygon
    // ################################################################

    void addPoi(std::string poiId, std::string poiType, const TraCIColor& color, int32_t layer, const Coord& pos);


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

    uint8_t genericGetUnsignedByte(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);                // new command
    Coord genericGetCoordv2(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);                       // new command
    std::vector<double> genericGetBoundingBox(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);     // new command
    uint8_t* genericGetArrayUnsignedInt(uint8_t, std::string, uint8_t, uint8_t);                                                    // new command
};

}

#endif

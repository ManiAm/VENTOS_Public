/****************************************************************************/
/// @file    TraCICommands.h
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

#include <chrono>
#include <ctime>
#include <ratio>

#undef ev
#include "boost/filesystem.hpp"

#include "baseAppl/03_BaseApplLayer.h"
#include "traci/TraCIConnection.h"
#include "traci/TraCIBuffer.h"
#include "mobility/TraCICoord.h"
#include "mobility/Coord.h"
#include "global/Color.h"
#include "global/GlobalConsts.h"


namespace VENTOS {

typedef struct simBoundary
{
    double x1;
    double y1;

    double x2;
    double y2;
} simBoundary_t;

typedef struct bestLanesEntry
{
    std::string laneId;
    double length;
    double occupation;
    int offset;
    int continuingDrive;
    std::vector<std::string> best;
} bestLanesEntry_t;

typedef struct linkEntry
{
    std::string consecutive1;
    std::string consecutive2;
    int priority;
    int opened;
    int approachingFoe;
    std::string state;
    std::string direction;
    double length;
} linkEntry_t;

typedef struct TL_info
{
    std::string TLS_id;
    uint32_t TLS_link_index;
    double TLS_distance;
    uint8_t linkState;
} TL_info_t;

// loop detector
typedef struct vehLD
{
    std::string vehID;
    double vehLength;
    double entryTime;
    double leaveTime;
    std::string vehType;
} vehLD_t;

typedef struct leader
{
    std::string leaderID;
    double distance2Leader;
} leader_t;

typedef enum carFollowingModel {
    SUMO_CF_KRAUSS = 0,
    SUMO_CF_KRAUSS_PLUS_SLOPE,
    SUMO_CF_KRAUSS_ORIG1,
    SUMO_CF_SMART_SK,
    SUMO_CF_DANIEL1,
    SUMO_CF_IDM,
    SUMO_CF_IDMM,
    SUMO_CF_PWAGNER2009,
    SUMO_CF_BKERNER,
    SUMO_CF_WIEDEMANN,

    SUMO_CF_OPTIMALSPEED,
    SUMO_CF_KRAUSSFIXED,
    SUMO_CF_ACC,
    SUMO_CF_CACC,

    SUMO_CF_MAX
} carFollowingModel_t;


typedef enum CFMODES {
    Mode_Undefined,
    Mode_NoData,
    Mode_DataLoss,
    Mode_SpeedControl,
    Mode_GapControl,
    Mode_EmergencyBrake,
    Mode_Stopped
} CFMODES_t;

typedef enum VehicleSignal {
    VEH_SIGNAL_UNDEF = -1,
    /// @brief Everything is switched off
    VEH_SIGNAL_NONE = 0,
    /// @brief Right blinker lights are switched on
    VEH_SIGNAL_BLINKER_RIGHT = 1,
    /// @brief Left blinker lights are switched on
    VEH_SIGNAL_BLINKER_LEFT = 2,
    /// @brief Blinker lights on both sides are switched on
    VEH_SIGNAL_BLINKER_EMERGENCY = 4,
    /// @brief The brake lights are on
    VEH_SIGNAL_BRAKELIGHT = 8,
    /// @brief The front lights are on (no visualisation)
    VEH_SIGNAL_FRONTLIGHT = 16,
    /// @brief The fog lights are on (no visualisation)
    VEH_SIGNAL_FOGLIGHT = 32,
    /// @brief The high beam lights are on (no visualisation)
    VEH_SIGNAL_HIGHBEAM = 64,
    /// @brief The backwards driving lights are on (no visualisation)
    VEH_SIGNAL_BACKDRIVE = 128,
    /// @brief The wipers are on
    VEH_SIGNAL_WIPER = 256,
    /// @brief One of the left doors is opened
    VEH_SIGNAL_DOOR_OPEN_LEFT = 512,
    /// @brief One of the right doors is opened
    VEH_SIGNAL_DOOR_OPEN_RIGHT = 1024,
    /// @brief A blue emergency light is on
    VEH_SIGNAL_EMERGENCY_BLUE = 2048,
    /// @brief A red emergency light is on
    VEH_SIGNAL_EMERGENCY_RED = 4096,
    /// @brief A yellow emergency light is on
    VEH_SIGNAL_EMERGENCY_YELLOW = 8192
} VehicleSignal_t;

typedef struct colorVal
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
} colorVal_t;

typedef struct departureArrivalEntry
{
    double departure;
    double arrival;
} departureArrivalEntry_t;

typedef struct
{
    uint32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t hour;
    uint32_t minute;
    uint32_t second;
    uint32_t millisecond;
} date_t;


class TraCI_Commands : public BaseApplLayer
{
public:
    // list of vehicles removed from the simulation with vehicleRemove method
    // this is necessary to circumvent the SUMO bug in getting the loaded vehicles list after vehicleRemove
    std::vector<std::string> removed_vehicles;

    bool TraCIclosedOnError = true;

    // record the departure and arrival time of each vehicle
    std::map<std::string /*SUMOID*/, departureArrivalEntry_t> departureArrival;

    // all hosts managed by us
    std::map<std::string /*SUMOID*/, cModule*> hosts;

private:
    typedef omnetpp::cSimpleModule super;

    typedef std::chrono::high_resolution_clock::time_point Htime_t;

protected:
    double updateInterval = -1;
    TraCIConnection* connection = NULL;

    // network boundaries
    TraCICoord netbounds1;
    TraCICoord netbounds2;
    int margin;

    bool equilibrium_vehicle = false;

    // start/end/duration of simulation
    std::chrono::milliseconds simStartTime;
    std::chrono::milliseconds simEndTime;

    // logging TraCI command exchange
    typedef struct TraCIcommandEntry
    {
        double timeStamp;
        Htime_t sentAt;
        Htime_t completeAt;
        std::string senderModule;
        uint8_t commandGroupId;
        uint8_t commandId;
        std::string commandName;
    } TraCIcommandEntry_t;

    bool record_TraCI_activity;
    std::vector<TraCIcommandEntry_t> exchangedTraCIcommands;

    // storing the mapping between vehicle ids and the corresponding SUMO ids
    std::map<std::string /*veh SUMO id*/, std::string /*veh OMNET id*/> SUMOid_OMNETid_mapping;
    std::map<std::string /*veh OMNET id*/, std::string /*veh SUMO id*/> OMNETid_SUMOid_mapping;

    // storing the mapping between emulated vehicle ids and the corresponding HIL board ipv4 address
    std::map<std::string /*veh SUMO id*/, std::string /*ip address*/> SUMOid_ipv4_mapping;
    std::map<std::string /*ip address*/, std::string /*veh SUMO id*/> ipv4_SUMOid_mapping;

public:
    virtual ~TraCI_Commands();
    virtual void initialize(int stage);
    virtual void handleMessage(omnetpp::cMessage *msg);
    virtual void finish();

    static TraCI_Commands * getTraCI();

    // ################################################################
    //                          subscription
    // ################################################################

    // CMD_SUBSCRIBE_SIM_VARIABLE
    TraCIBuffer subscribeSimulation(uint32_t beginTime, uint32_t endTime, std::string objectId, std::vector<uint8_t> variables);
    // CMD_SUBSCRIBE_VEHICLE_VARIABLE
    TraCIBuffer subscribeVehicle(uint32_t beginTime, uint32_t endTime, std::string objectId, std::vector<uint8_t> variables);
    // CMD_SUBSCRIBE_PERSON_VARIABLE
    TraCIBuffer subscribePerson(uint32_t beginTime, uint32_t endTime, std::string objectId, std::vector<uint8_t> variables);

    // ################################################################
    //                            simulation
    // ################################################################

    // CMD_GET_SIM_VARIABLE
    uint32_t simulationGetLoadedVehiclesCount();
    uint32_t simulationGetDepartedVehiclesCount();
    uint32_t simulationGetArrivedNumber();
    uint32_t simulationGetMinExpectedNumber();
    std::vector<std::string> simulationGetLoadedVehiclesIDList();
    simBoundary_t simulationGetNetBoundary();
    uint32_t simulationGetDelta();  // delta is time-step length
    uint32_t simulationGetCurrentTime();

    date_t simulationGetStartTime();     // new command [returns the simulation start time]
    date_t simulationGetEndTime();       // new command [returns the simulation stop time]
    date_t simulationGetDuration();      // new command [returns the duration of the simulation]

    std::string simulationGetStartTime_str();     // new command [returns the simulation start time in string]
    std::string simulationGetEndTime_str();       // new command [returns the simulation stop time in string]
    std::string simulationGetDuration_str();      // new command [returns the duration of the simulation in string]

    uint32_t simulationGetEndingTeleportedVehicleCount();
    std::vector<std::string> simulationGetEndingTeleportedVehicleIDList();

    void simulationTerminate(bool TraCIclosed = false);  // new command [terminate the simulation]

    bool simulationIsEquilibriumActive();

    // ################################################################
    //                            vehicle
    // ################################################################

    // CMD_GET_VEHICLE_VARIABLE
    bool vehicleCouldChangeLane(std::string nodeId, int direction);
    std::vector<std::string> vehicleGetIDList();
    uint32_t vehicleGetIDCount();
    double vehicleGetSpeed(std::string);
    double vehicleGetAngle(std::string);
    uint8_t vehicleGetStopState(std::string);
    TraCICoord vehicleGetPosition(std::string);
    std::string vehicleGetEdgeID(std::string);
    std::string vehicleGetLaneID(std::string);
    uint32_t vehicleGetLaneIndex(std::string);
    double vehicleGetLanePosition(std::string);
    std::string vehicleGetTypeID(std::string);
    std::string vehicleGetRouteID(std::string);
    std::vector<std::string> vehicleGetRoute(std::string);
    uint32_t vehicleGetRouteIndex(std::string);
    double vehicleGetDrivingDistance(std::string);
    std::map<int,bestLanesEntry_t> vehicleGetBestLanes(std::string);
    colorVal_t vehicleGetColor(std::string);
    VehicleSignal_t vehicleGetSignalStatus(std::string);
    double vehicleGetLength(std::string);
    double vehicleGetMinGap(std::string);
    double vehicleGetWaitingTime(std::string);
    double vehicleGetMaxSpeed(std::string);
    double vehicleGetMaxAccel(std::string);
    double vehicleGetMaxDecel(std::string);
    double vehicleGetTimeGap(std::string);
    std::string vehicleGetClass(std::string);
    leader_t vehicleGetLeader(std::string, double);
    std::vector<TL_info_t> vehicleGetNextTLS(std::string);
    double vehicleGetCO2Emission(std::string);
    double vehicleGetCOEmission(std::string);
    double vehicleGetHCEmission(std::string);
    double vehicleGetPMxEmission(std::string);
    double vehicleGetNOxEmission(std::string);
    double vehicleGetNoiseEmission(std::string);
    double vehicleGetFuelConsumption(std::string);
    std::string vehicleGetEmissionClass(std::string);
    double vehicleGetCurrentAccel(std::string);          // new command
    double vehicleGetDepartureTime(std::string);         // new command
    double vehicleGetArrivalTime(std::string);           // new command
    std::string vehicleGetCarFollowingModelName(std::string);         // new command [returns the car-following model name -- ACC/CACC]
    carFollowingModel_t vehicleGetCarFollowingModelID(std::string);   // new command [returns the car-following model number -- 0(ACC)/1(CACC)]
    int vehicleGetCACCStrategy(std::string);                          // new command [returns the CACC strategy number]
    CFMODES_t vehicleGetCarFollowingModelMode(std::string);           // new command [returns the current ACC/CACC car following mode]

    std::string vehicleGetPlatoonId(std::string);    // new command
    int vehicleGetPlatoonDepth(std::string);    // new command

    bool vehicleExist(std::string);

    // CMD_SET_VEHICLE_VARIABLE
    void vehicleSetStop(std::string, std::string, double, uint8_t, int32_t, uint8_t);
    void vehicleResume(std::string);
    void vehicleSetSpeed(std::string, double);
    void vehicleSetSpeedMode(std::string, uint32_t bitset);
    int32_t vehicleBuildLaneChangeMode(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
    void vehicleSetLaneChangeMode(std::string, int32_t);
    void vehicleChangeLane(std::string, uint8_t, double);
    void vehicleSetRoute(std::string, std::list<std::string> value);
    void vehicleSetRouteID(std::string, std::string);
    void vehicleChangeTarget(std::string, std::string);
    void vehicleMoveTo(std::string, std::string, double);
    void vehicleSetColor(std::string, const colorVal_t);
    void vehicleSetColor(std::string, const RGB);
    void vehicleSetClass(std::string, std::string);
    void vehicleSetLength(std::string, double);
    void vehicleSetWidth(std::string, double);
    void vehicleSlowDown(std::string, double, int);
    void vehicleSetSignalStatus(std::string, int32_t);
    void vehicleSetMaxSpeed(std::string, double);
    void vehicleSetMaxAccel(std::string, double);
    void vehicleSetMaxDecel(std::string, double);
    void vehicleSetTimeGap(std::string, double);
    void vehicleAdd(std::string, std::string, std::string, int32_t, double, double, uint8_t);
    void vehicleRemove(std::string, uint8_t);
    void vehicleSetErrorGap(std::string, double);                   // new command [set an error value for the gap]
    void vehicleSetErrorRelSpeed(std::string, double);              // new command [set an error value for relative speed]
    void vehicleSetDebug(std::string, bool);                        // new command [should the debug info be printed in the SUMO output console ?]
    void vehicleSetVint(std::string, double);                       // new command
    void vehicleSetComfAccel(std::string, double);                  // new command
    void vehicleSetComfDecel(std::string, double);                  // new command

    // used by the protocol
    void vehiclePlatoonInit(std::string, std::deque<std::string>);
    void vehiclePlatoonViewUpdate(std::string, std::string);

    // ################################################################
    //                          vehicle type
    // ################################################################

    // CMD_GET_VEHICLETYPE_VARIABLE
    std::vector<std::string> vehicleTypeGetIDList();
    uint32_t vehicleTypeGetIDCount();
    double vehicleTypeGetLength(std::string);
    double vehicleTypeGetMaxSpeed(std::string);
    double vehicleTypeGetMinGap(std::string);

    // ################################################################
    //                              route
    // ################################################################

    // CMD_GET_ROUTE_VARIABLE
    std::vector<std::string> routeGetIDList();
    uint32_t routeGetIDCount();
    std::vector<std::string> routeGetEdges(std::string);
    std::vector<std::string> routeShortest(std::string, std::string);  // new command

    // CMD_SET_ROUTE_VARIABLE
    void routeAdd(std::string, std::vector<std::string>);

    // ################################################################
    //                              edge
    // ################################################################

    // CMD_GET_EDGE_VARIABLE
    std::vector<std::string> edgeGetIDList();
    uint32_t edgeGetIDCount();
    double edgeGetMeanTravelTime(std::string);
    uint32_t edgeGetLastStepVehicleNumber(std::string);
    std::vector<std::string> edgeGetLastStepVehicleIDs(std::string);
    double edgeGetLastStepMeanVehicleSpeed(std::string);
    double edgeGetLastStepMeanVehicleLength(std::string);
    std::vector<std::string> edgeGetLastStepPersonIDs(std::string);

    uint32_t edgeGetLaneCount(std::string);                                   // new command
    std::vector<std::string> edgeGetAllowedLanes(std::string, std::string);   // new command

    // CMD_SET_EDGE_VARIABLE
    void edgeSetGlobalTravelTime(std::string, int32_t, int32_t, double);

    // ################################################################
    //                              lane
    // ################################################################

    // CMD_GET_LANE_VARIABLE
    std::vector<std::string> laneGetIDList();
    uint32_t laneGetIDCount();
    uint8_t laneGetLinkNumber(std::string);
    std::map<int,linkEntry_t> laneGetLinks(std::string);
    std::vector<std::string> laneGetAllowedClasses(std::string);
    std::string laneGetEdgeID(std::string);
    double laneGetLength(std::string);
    double laneGetMaxSpeed(std::string);
    double laneGetWidth(std::string);
    uint32_t laneGetLastStepVehicleNumber(std::string);
    std::vector<std::string> laneGetLastStepVehicleIDs(std::string);
    double laneGetLastStepMeanVehicleSpeed(std::string);
    double laneGetLastStepMeanVehicleLength(std::string);

    // CMD_SET_LANE_VARIABLE
    void laneSetMaxSpeed(std::string, double);

    // ################################################################
    //                 loop detector (E1-Detectors)
    // ################################################################

    // CMD_GET_INDUCTIONLOOP_VARIABLE
    std::vector<std::string> LDGetIDList();
    uint32_t LDGetIDCount(std::string);
    std::string LDGetLaneID(std::string);
    double LDGetPosition(std::string);
    uint32_t LDGetLastStepVehicleNumber(std::string);
    std::vector<std::string> LDGetLastStepVehicleIDs(std::string);
    double LDGetLastStepMeanVehicleSpeed(std::string);
    double LDGetElapsedTimeLastDetection(std::string);
    std::vector<vehLD_t> LDGetLastStepVehicleData(std::string);
    double LDGetLastStepOccupancy(std::string);

    // ################################################################
    //                lane area detector (E2-Detectors)
    // ################################################################

    // CMD_GET_AREAL_DETECTOR_VARIABLE
    std::vector<std::string> LADGetIDList();
    uint32_t LADGetIDCount(std::string);
    std::string LADGetLaneID(std::string);
    uint32_t LADGetLastStepVehicleNumber(std::string);
    std::vector<std::string> LADGetLastStepVehicleIDs(std::string);
    double LADGetLastStepMeanVehicleSpeed(std::string);
    uint32_t LADGetLastStepVehicleHaltingNumber(std::string);
    double LADGetLastStepJamLengthInMeter(std::string);

    // ################################################################
    //                          traffic light
    // ################################################################

    // CMD_GET_TL_VARIABLE
    std::vector<std::string> TLGetIDList();
    uint32_t TLGetIDCount();
    std::vector<std::string> TLGetControlledLanes(std::string);
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
    std::vector<std::string> junctionGetIDList();
    uint32_t junctionGetIDCount();
    TraCICoord junctionGetPosition(std::string);

    // ################################################################
    //                               GUI
    // ################################################################

    // CMD_GET_GUI_VARIABLE
    TraCICoord GUIGetOffset(std::string);
    std::vector<double> GUIGetBoundry(std::string);
    double GUIGetZoom(std::string);

    // CMD_SET_GUI_VARIABLE
    void GUISetZoom(std::string, double);
    void GUISetOffset(std::string, double, double);
    void GUITakeScreenshot(std::string, std::string);
    void GUISetTrackVehicle(std::string, std::string);
    void GUIAddView(std::string);                                      // new command

    // ################################################################
    //                               polygon
    // ################################################################

    // CMD_GET_POLYGON_VARIABLE
    std::vector<std::string> polygonGetIDList();
    uint32_t polygonGetIDCount();
    std::vector<TraCICoord> polygonGetShape(std::string);
    std::string polygonGetTypeID(std::string);

    // CMD_SET_POLYGON_VARIABLE
    void polygonAdd(std::string, std::string, const RGB, bool, int32_t, const std::list<Coord>&);
    void polygonAdd(std::string, std::string, const RGB, bool, int32_t, const std::list<TraCICoord>&);
    void polygonSetFilled(std::string, uint8_t);

    // ################################################################
    //                               POI
    // ################################################################

    void poiAdd(std::string, std::string, const RGB, int32_t, const TraCICoord&);
    void poiAdd(std::string, std::string, const RGB, int32_t, const Coord&);


    // ################################################################
    //                               person
    // ################################################################

    // CMD_GET_PERSON
    std::vector<std::string> personGetIDList();
    uint32_t personGetIDCount();
    std::string personGetTypeID(std::string);
    TraCICoord personGetPosition(std::string);
    double personGetAngle(std::string);
    std::string personGetEdgeID(std::string);
    double personGetEdgePosition(std::string);
    double personGetSpeed(std::string);
    std::string personGetNextEdge(std::string);

    // CMD_SET_PERSON
    void personAdd(std::string, std::string, double, int, std::string);

    // ################################################################
    //                              RSU
    // ################################################################

    std::vector<std::string> rsuGetIDList();
    uint32_t rsuGetIDCount();
    TraCICoord rsuGetPosition(std::string);

    // ################################################################
    //                          Vehicle Platoon
    // ################################################################

    std::vector<std::string> platoonGetIDList();
    uint32_t platoonGetIDCount();
    omnetpp::cModule* platoonGetLeaderModule(std::string);
    uint32_t platoonGetSize(std::string);
    uint32_t platoonGetOptSize(std::string);
    uint32_t platoonGetMaxSize(std::string);
    std::vector<std::string> platoonGetMembers(std::string);
    bool platoonIsPltMgmtProtActive(std::string);

    // ################################################################
    //                              Obstacle
    // ################################################################

    std::vector<std::string> obstacleGetIDList();
    uint32_t obstacleGetIDCount();
    TraCICoord obstacleGetPosition(std::string);
    std::string obstacleGetEdgeID(std::string);
    std::string obstacleGetLaneID(std::string);
    uint32_t obstacleGetLaneIndex(std::string);
    double obstacleGetLanePosition(std::string);

    // ################################################################
    //                      SUMO-OMNET conversion
    // ################################################################

    std::string convertId_traci2omnet(std::string SUMOid) const;   // convert SUMO id to OMNET++ id
    std::string convertId_omnet2traci(std::string omnetid) const;  // convert OMNET++ id to SUMO id

    Coord convertCoord_traci2omnet(TraCICoord coord) const;  // convert TraCI coordinates to OMNeT++ coordinates
    TraCICoord convertCoord_omnet2traci(Coord coord) const;  // convert OMNeT++ coordinates to TraCI coordinates

    double convertAngle_traci2omnet(double angle) const;  // convert TraCI angle to OMNeT++ angle (in rad)
    double convertAngle_omnet2traci(double angle) const;  // convert OMNeT++ angle (in rad) to TraCI angle

    // ################################################################
    //                       SUMO directory
    // ################################################################

    boost::filesystem::path getFullPath_SUMOApplication();
    boost::filesystem::path getFullPath_SUMOConfig();

    bool IsGUI();

    // ################################################################
    //                    Hardware in the loop (HIL)
    // ################################################################

    std::string ip2vehicleId(std::string ip) const;   // HIL ipv4 address --> SUMO id of emulated vehicle
    std::string vehicleId2ip(std::string vID) const;  // SUMO id of emulated vehicle --> HIL ipv4 address

    virtual bool isEmulated(std::string vID) {
        throw omnetpp::cRuntimeError("derived class should implement this method!");
    };
    virtual void emulatedAdd(std::string vID, std::string ip, std::string = "") {
        throw omnetpp::cRuntimeError("derived class should implement this method!");
    };
    virtual void emulatedRemove(std::string vID, std::string = "") {
        throw omnetpp::cRuntimeError("derived class should implement this method!");
    };

    // ################################################################
    //                            Mapping
    // ################################################################

    void addMapping(std::string SUMOID, std::string OMNETID);
    void removeMapping(std::string SUMOID);

    void addMappingEmulated(std::string ip, std::string vID);
    void removeMappingEmulated(std::string vID);

protected:

    // these methods are not meant to be exposed to the user!

    std::pair<uint32_t, std::string> getVersion();
    void close_TraCI_connection();
    std::pair<TraCIBuffer, uint32_t> simulationTimeStep(uint32_t targetTime);

    void recordDeparture(std::string SUMOID);
    void recordArrival(std::string SUMOID);

private:
    // ################################################################
    //                    generic methods for getters
    // ################################################################

    double genericGetDouble(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
    int32_t genericGetInt(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
    std::string genericGetString(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
    std::vector<std::string> genericGetStringVector(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
    TraCICoord genericGetCoord(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
    std::vector<TraCICoord> genericGetCoordVector(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
    uint8_t genericGetUnsignedByte(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
    std::vector<double> genericGetBoundingBox(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);

    enum action_t
    {
        commandStart,
        commandComplete,
    };

    // logging TraCI exchange
    void record_TraCI_activity_func(action_t, uint8_t, uint8_t, std::string);
    void save_TraCI_activity_toFile();
    void save_TraCI_activity_summary_toFile();
};

}

#endif

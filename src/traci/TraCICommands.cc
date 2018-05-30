/****************************************************************************/
/// @file    TraCICommands.cc
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

#include <cmath>
#include <iomanip>
#include <algorithm>
#include <regex>
#include "boost/format.hpp"

#undef ev
#include "boost/filesystem.hpp"

#include "traci/TraCICommands.h"
#include "traci/TraCIConstants.h"
#include "nodes/vehicle/Manager.h"

namespace VENTOS {

#define PARAMS_DELIM  "#"

Define_Module(VENTOS::TraCI_Commands);

TraCI_Commands::~TraCI_Commands()
{

}


void TraCI_Commands::initialize(int stage)
{
    super::initialize(stage);

    if (stage == 0)
    {
        record_TraCI_activity = par("record_TraCI_activity").boolValue();
        margin = par("margin").intValue();

        if(par("active").boolValue())
        {
            simStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

            // set initial value for end time
            simEndTime = simStartTime;
        }
    }
}


void TraCI_Commands::finish()
{
    // record simulation end time
    simEndTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    save_TraCI_activity_toFile();
    save_TraCI_activity_summary_toFile();
}


void TraCI_Commands::handleMessage(omnetpp::cMessage *msg)
{
    throw omnetpp::cRuntimeError("Can't handle msg '%s' of kind '%d'", msg->getFullName(), msg->getKind());
}


TraCI_Commands * TraCI_Commands::getTraCI()
{
    // get a pointer to the TraCI module
    omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
    // make sure module TraCI exists
    ASSERT(module);

    // get a pointer to TraCI class
    TraCI_Commands *TraCI = static_cast<TraCI_Commands *>(module);
    // make sure the TraCI pointer is not null
    ASSERT(TraCI);

    return TraCI;
}


// ################################################################
//                            subscription
// ################################################################

// CMD_SUBSCRIBE_SIM_VARIABLE
TraCIBuffer TraCI_Commands::subscribeSimulation(uint32_t beginTime, uint32_t endTime, std::string objectId, std::vector<uint8_t> variables)
{
    record_TraCI_activity_func(commandStart, CMD_SUBSCRIBE_SIM_VARIABLE, 0xff, "simulationSubscribe");

    TraCIBuffer p;
    p << beginTime << endTime << objectId << (uint8_t)variables.size();
    for(uint8_t i : variables)
        p << i;

    TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_SIM_VARIABLE, p);

    record_TraCI_activity_func(commandComplete, CMD_SUBSCRIBE_SIM_VARIABLE, 0xff, "simulationSubscribe");

    return buf;
}


// CMD_SUBSCRIBE_VEHICLE_VARIABLE
TraCIBuffer TraCI_Commands::subscribeVehicle(uint32_t beginTime, uint32_t endTime, std::string objectId, std::vector<uint8_t> variables)
{
    record_TraCI_activity_func(commandStart, CMD_SUBSCRIBE_VEHICLE_VARIABLE, 0xff, "vehicleSubscribe");

    TraCIBuffer p;
    p << beginTime << endTime << objectId << (uint8_t)variables.size();
    for(uint8_t i : variables)
        p << i;

    TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_VEHICLE_VARIABLE, p);

    record_TraCI_activity_func(commandComplete, CMD_SUBSCRIBE_VEHICLE_VARIABLE, 0xff, "vehicleSubscribe");

    return buf;
}


// CMD_SUBSCRIBE_PERSON_VARIABLE
TraCIBuffer TraCI_Commands::subscribePerson(uint32_t beginTime, uint32_t endTime, std::string objectId, std::vector<uint8_t> variables)
{
    record_TraCI_activity_func(commandStart, CMD_SUBSCRIBE_PERSON_VARIABLE, 0xff, "subscribePerson");

    TraCIBuffer p;
    p << beginTime << endTime << objectId << (uint8_t)variables.size();
    for(uint8_t i : variables)
        p << i;

    TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_PERSON_VARIABLE, p);

    record_TraCI_activity_func(commandComplete, CMD_SUBSCRIBE_PERSON_VARIABLE, 0xff, "subscribePerson");

    return buf;
}


// ################################################################
//                            simulation
// ################################################################

// ####################
// CMD_GET_SIM_VARIABLE
// ####################

uint32_t TraCI_Commands::simulationGetLoadedVehiclesCount()
{
    record_TraCI_activity_func(commandStart, CMD_GET_SIM_VARIABLE, VAR_LOADED_VEHICLES_NUMBER, "simulationGetLoadedVehiclesCount");

    TraCIBuffer buf = connection->query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_LOADED_VEHICLES_NUMBER) << std::string("sim0"));

    uint8_t cmdLength_resp; buf >> cmdLength_resp;
    uint8_t commandId_resp; buf >> commandId_resp;
    ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
    uint8_t variableId_resp; buf >> variableId_resp;
    ASSERT(variableId_resp == VAR_LOADED_VEHICLES_NUMBER);
    std::string simId; buf >> simId;
    uint8_t typeId_resp; buf >> typeId_resp;
    ASSERT(typeId_resp == TYPE_INTEGER);

    uint32_t val;
    buf >> val;

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_SIM_VARIABLE, VAR_LOADED_VEHICLES_NUMBER, "simulationGetLoadedVehiclesCount");

    return val;
}


std::vector<std::string> TraCI_Commands::simulationGetLoadedVehiclesIDList()
{
    record_TraCI_activity_func(commandStart, CMD_GET_SIM_VARIABLE, VAR_LOADED_VEHICLES_IDS, "simulationGetLoadedVehiclesIDList");

    uint8_t resultTypeId = TYPE_STRINGLIST;
    std::vector<std::string> res;

    TraCIBuffer buf = connection->query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_LOADED_VEHICLES_IDS) << std::string("sim0"));

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == RESPONSE_GET_SIM_VARIABLE);
    uint8_t varId; buf >> varId;
    ASSERT(varId == VAR_LOADED_VEHICLES_IDS);
    std::string objectId_r; buf >> objectId_r;
    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint32_t count; buf >> count;
    for (uint32_t i = 0; i < count; i++) {
        std::string id; buf >> id;
        res.push_back(id);
    }

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_SIM_VARIABLE, VAR_LOADED_VEHICLES_IDS, "simulationGetLoadedVehiclesIDList");

    return res;
}


uint32_t TraCI_Commands::simulationGetDepartedVehiclesCount()
{
    record_TraCI_activity_func(commandStart, CMD_GET_SIM_VARIABLE, VAR_DEPARTED_VEHICLES_NUMBER, "simulationGetDepartedVehiclesCount");

    TraCIBuffer buf = connection->query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_DEPARTED_VEHICLES_NUMBER) << std::string("sim0"));

    uint8_t cmdLength_resp; buf >> cmdLength_resp;
    uint8_t commandId_resp; buf >> commandId_resp;
    ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
    uint8_t variableId_resp; buf >> variableId_resp;
    ASSERT(variableId_resp == VAR_DEPARTED_VEHICLES_NUMBER);
    std::string simId; buf >> simId;
    uint8_t typeId_resp; buf >> typeId_resp;
    ASSERT(typeId_resp == TYPE_INTEGER);

    uint32_t val;
    buf >> val;

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_SIM_VARIABLE, VAR_DEPARTED_VEHICLES_NUMBER, "simulationGetDepartedVehiclesCount");

    return val;
}


simBoundary_t TraCI_Commands::simulationGetNetBoundary()
{
    record_TraCI_activity_func(commandStart, CMD_GET_SIM_VARIABLE, VAR_NET_BOUNDING_BOX, "simulationGetNetBoundary");

    // query road network boundaries
    TraCIBuffer buf = connection->query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_NET_BOUNDING_BOX) << std::string("sim0"));

    uint8_t cmdLength_resp; buf >> cmdLength_resp;
    uint8_t commandId_resp; buf >> commandId_resp;
    ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
    uint8_t variableId_resp; buf >> variableId_resp;
    ASSERT(variableId_resp == VAR_NET_BOUNDING_BOX);
    std::string simId; buf >> simId;
    uint8_t typeId_resp; buf >> typeId_resp;
    ASSERT(typeId_resp == TYPE_BOUNDINGBOX);

    simBoundary_t boundary = {};

    buf >> boundary.x1;
    buf >> boundary.y1;
    buf >> boundary.x2;
    buf >> boundary.y2;

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_SIM_VARIABLE, VAR_NET_BOUNDING_BOX, "simulationGetNetBoundary");

    return boundary;
}


uint32_t TraCI_Commands::simulationGetMinExpectedNumber()
{
    record_TraCI_activity_func(commandStart, CMD_GET_SIM_VARIABLE, VAR_MIN_EXPECTED_VEHICLES, "simulationGetMinExpectedNumber");

    // query road network boundaries
    TraCIBuffer buf = connection->query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_MIN_EXPECTED_VEHICLES) << std::string("sim0"));

    uint8_t cmdLength_resp; buf >> cmdLength_resp;
    uint8_t commandId_resp; buf >> commandId_resp;
    ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
    uint8_t variableId_resp; buf >> variableId_resp;
    ASSERT(variableId_resp == VAR_MIN_EXPECTED_VEHICLES);
    std::string simId; buf >> simId;
    uint8_t typeId_resp; buf >> typeId_resp;
    ASSERT(typeId_resp == TYPE_INTEGER);

    uint32_t val;
    buf >> val;

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_SIM_VARIABLE, VAR_MIN_EXPECTED_VEHICLES, "simulationGetMinExpectedNumber");

    return val;
}


uint32_t TraCI_Commands::simulationGetArrivedNumber()
{
    record_TraCI_activity_func(commandStart, CMD_GET_SIM_VARIABLE, VAR_ARRIVED_VEHICLES_NUMBER, "simulationGetArrivedNumber");

    // query road network boundaries
    TraCIBuffer buf = connection->query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_ARRIVED_VEHICLES_NUMBER) << std::string("sim0"));

    uint8_t cmdLength_resp; buf >> cmdLength_resp;
    uint8_t commandId_resp; buf >> commandId_resp;
    ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
    uint8_t variableId_resp; buf >> variableId_resp;
    ASSERT(variableId_resp == VAR_ARRIVED_VEHICLES_NUMBER);
    std::string simId; buf >> simId;
    uint8_t typeId_resp; buf >> typeId_resp;
    ASSERT(typeId_resp == TYPE_INTEGER);

    uint32_t val;
    buf >> val;

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_SIM_VARIABLE, VAR_ARRIVED_VEHICLES_NUMBER, "simulationGetArrivedNumber");

    return val;
}


uint32_t TraCI_Commands::simulationGetDelta()
{
    // do not ask SUMO if we already know the time step
    // this is also a workaround to call simulationGetDelta() in finish()
    if(updateInterval != -1)
        return (uint32_t)(updateInterval * 1000);

    record_TraCI_activity_func(commandStart, CMD_GET_SIM_VARIABLE, VAR_DELTA_T, "simulationGetDelta");

    TraCIBuffer buf = connection->query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_DELTA_T) << std::string("sim0"));

    uint8_t cmdLength_resp; buf >> cmdLength_resp;
    uint8_t commandId_resp; buf >> commandId_resp;
    ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
    uint8_t variableId_resp; buf >> variableId_resp;
    ASSERT(variableId_resp == VAR_DELTA_T);
    std::string simId; buf >> simId;
    uint8_t typeId_resp; buf >> typeId_resp;
    ASSERT(typeId_resp == TYPE_INTEGER);

    uint32_t val;
    buf >> val;

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_SIM_VARIABLE, VAR_DELTA_T, "simulationGetDelta");

    return val;
}


uint32_t TraCI_Commands::simulationGetCurrentTime()
{
    record_TraCI_activity_func(commandStart, CMD_GET_SIM_VARIABLE, VAR_TIME_STEP, "simulationGetCurrentTime");

    TraCIBuffer buf = connection->query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_TIME_STEP) << std::string("sim0"));

    uint8_t cmdLength_resp; buf >> cmdLength_resp;
    uint8_t commandId_resp; buf >> commandId_resp;
    ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
    uint8_t variableId_resp; buf >> variableId_resp;
    ASSERT(variableId_resp == VAR_TIME_STEP);
    std::string simId; buf >> simId;
    uint8_t typeId_resp; buf >> typeId_resp;
    ASSERT(typeId_resp == TYPE_INTEGER);

    uint32_t val;
    buf >> val;

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_SIM_VARIABLE, VAR_TIME_STEP, "simulationGetCurrentTime");

    return val;
}


date_t TraCI_Commands::simulationGetStartTime()
{
    // seconds since the epoch in seconds
    std::time_t t = std::chrono::duration_cast<std::chrono::seconds>(simStartTime).count();

    // convert to local time
    struct tm *timeinfo = std::localtime(&t);

    date_t startTime = {};

    // save it into our own structure
    startTime.year = timeinfo->tm_year + 1900;
    startTime.month = timeinfo->tm_mon + 1;
    startTime.day = timeinfo->tm_mday;
    startTime.hour = timeinfo->tm_hour;
    startTime.minute = timeinfo->tm_min;
    startTime.second = timeinfo->tm_sec;
    startTime.millisecond = simStartTime.count() % 1000; // fractional_seconds

    return startTime;
}


date_t TraCI_Commands::simulationGetEndTime()
{
    // seconds since the epoch in seconds
    std::time_t t = std::chrono::duration_cast<std::chrono::seconds>(simEndTime).count();

    // convert to local time
    struct tm *timeinfo = std::localtime(&t);

    date_t endTime = {};

    // save it into our own structure
    endTime.year = timeinfo->tm_year + 1900;
    endTime.month = timeinfo->tm_mon + 1;
    endTime.day = timeinfo->tm_mday;
    endTime.hour = timeinfo->tm_hour;
    endTime.minute = timeinfo->tm_min;
    endTime.second = timeinfo->tm_sec;
    endTime.millisecond = simStartTime.count() % 1000; // fractional_seconds

    return endTime;
}


date_t TraCI_Commands::simulationGetDuration()
{
    std::chrono::duration<double, std::milli> fp_ms = simEndTime - simStartTime;
    int duration_ms = fp_ms.count();

    int seconds = (int) (duration_ms / 1000) % 60 ;
    int minutes = (int) ((duration_ms / (1000*60)) % 60);
    int hours   = (int) ((duration_ms / (1000*60*60)) % 24);

    date_t duration = {};

    duration.hour = hours;
    duration.minute = minutes;
    duration.second = seconds;

    return duration;
}


std::string TraCI_Commands::simulationGetStartTime_str()
{
    date_t startTime = simulationGetStartTime();

    std::ostringstream startTime_s;
    startTime_s << boost::format("%04u-%02u-%02u %02u:%02u:%02u.%03u") %
            (startTime.year) %
            (startTime.month) %
            (startTime.day) %
            (startTime.hour) %
            (startTime.minute) %
            (startTime.second) %
            (startTime.millisecond);

    return startTime_s.str();
}


std::string TraCI_Commands::simulationGetEndTime_str()
{
    date_t endTime = simulationGetEndTime();

    std::ostringstream endTime_s;
    endTime_s << boost::format("%04u-%02u-%02u %02u:%02u:%02u.%03u") %
            (endTime.year) %
            (endTime.month) %
            (endTime.day) %
            (endTime.hour) %
            (endTime.minute) %
            (endTime.second) %
            (endTime.millisecond);

    return endTime_s.str();
}


std::string TraCI_Commands::simulationGetDuration_str()
{
    date_t duration = simulationGetDuration();

    std::ostringstream duration_s;
    duration_s << boost::format("%1% hour, %2% minute, %3% second") %
            (duration.hour) %
            (duration.minute) %
            (duration.second);

    return duration_s.str();
}


uint32_t TraCI_Commands::simulationGetEndingTeleportedVehicleCount()
{
    record_TraCI_activity_func(commandStart, CMD_GET_SIM_VARIABLE, VAR_TELEPORT_ENDING_VEHICLES_NUMBER, "simulationGetEndingTeleportedVehicleCount");

    // query road network boundaries
    TraCIBuffer buf = connection->query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_TELEPORT_ENDING_VEHICLES_NUMBER) << std::string("sim0"));

    uint8_t cmdLength_resp; buf >> cmdLength_resp;
    uint8_t commandId_resp; buf >> commandId_resp;
    ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
    uint8_t variableId_resp; buf >> variableId_resp;
    ASSERT(variableId_resp == VAR_TELEPORT_ENDING_VEHICLES_NUMBER);
    std::string simId; buf >> simId;
    uint8_t typeId_resp; buf >> typeId_resp;
    ASSERT(typeId_resp == TYPE_INTEGER);

    uint32_t val;
    buf >> val;

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_SIM_VARIABLE, VAR_TELEPORT_ENDING_VEHICLES_NUMBER, "simulationGetEndingTeleportedVehicleCount");

    return val;
}


std::vector<std::string> TraCI_Commands::simulationGetEndingTeleportedVehicleIDList()
{
    record_TraCI_activity_func(commandStart, CMD_GET_SIM_VARIABLE, VAR_TELEPORT_ENDING_VEHICLES_IDS, "simulationGetEndingTeleportedVehiclesIDList");

    uint8_t resultTypeId = TYPE_STRINGLIST;
    std::vector<std::string> res;

    TraCIBuffer buf = connection->query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_TELEPORT_ENDING_VEHICLES_IDS) << std::string("sim0"));

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == RESPONSE_GET_SIM_VARIABLE);
    uint8_t varId; buf >> varId;
    ASSERT(varId == VAR_TELEPORT_ENDING_VEHICLES_IDS);
    std::string objectId_r; buf >> objectId_r;
    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint32_t count; buf >> count;
    for (uint32_t i = 0; i < count; i++) {
        std::string id; buf >> id;
        res.push_back(id);
    }

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_SIM_VARIABLE, VAR_TELEPORT_ENDING_VEHICLES_IDS, "simulationGetEndingTeleportedVehiclesIDList");

    return res;
}


void TraCI_Commands::simulationTerminate(bool error)
{
    Enter_Method("simulationTerminate()");

    // is used in TraCI_Start::finish()
    this->TraCIclosedOnError = error;

    // TraCI connection is closed in TraCI_Start::finish()
    endSimulation();
}


bool TraCI_Commands::simulationIsEquilibriumActive()
{
    return this->equilibrium_vehicle;
}


// ################################################################
//                            vehicle
// ################################################################

// #########################
// CMD_GET_VEHICLE_VARIABLE
// #########################

bool TraCI_Commands::vehicleCouldChangeLane(std::string nodeId, int direction)
{
    // is it a valid direction?
    if (direction == -1 || direction == 1)
    {
        // get required data
        std::string myEdge = vehicleGetEdgeID(nodeId);
        std::string vehClass = vehicleGetClass(nodeId);
        int currentLane = vehicleGetLaneIndex(nodeId);
        int numLanes = edgeGetLaneCount(myEdge);
        auto allowedLanes = edgeGetAllowedLanes(myEdge, vehClass);

        // set currentLane to be the lane in specified direction; -1 is right, 1 is left
        currentLane += direction;

        // check boundaries
        if (currentLane > numLanes || currentLane < 0)
            return false;

        // check if adjacent lanes matches the currentLane (in terms of vehicle class)
        auto ii = std::find(allowedLanes.begin(), allowedLanes.end(), myEdge + "_" + std::to_string(currentLane));

        // no matches
        if(ii == allowedLanes.end())
            return false;

        return true;
    }
    else
        throw omnetpp::cRuntimeError("Not a valid direction in vehicleCouldChangeLane");
}


// gets a list of all vehicles in the network (alphabetically!!!)
std::vector<std::string> TraCI_Commands::vehicleGetIDList()
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, ID_LIST, "vehicleGetIDList");

    auto result = genericGetStringVector(CMD_GET_VEHICLE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, ID_LIST, "vehicleGetIDList");

    return result;
}


uint32_t TraCI_Commands::vehicleGetIDCount()
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, ID_COUNT, "vehicleGetIDCount");

    int32_t result = genericGetInt(CMD_GET_VEHICLE_VARIABLE, "", ID_COUNT, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, ID_COUNT, "vehicleGetIDCount");

    return result;
}


double TraCI_Commands::vehicleGetSpeed(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_SPEED, "vehicleGetSpeed");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_SPEED, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_SPEED, "vehicleGetSpeed");

    return result;
}


double TraCI_Commands::vehicleGetAngle(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_ANGLE, "vehicleGetAngle");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ANGLE, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_ANGLE, "vehicleGetAngle");

    return result;
}

// value = 1 * stopped + 2  * parking + 4 * triggered + 8 * containerTriggered +
//        16 * busstop + 32 * containerstop

// isStopped           value & 1 == 1     whether the vehicle is stopped
// isStoppedParking    value & 2 == 2     whether the vehicle is parking (implies stopped)
// isStoppedTriggered  value & 12 > 0     whether the vehicle is stopped and waiting for a person or container
// isAtBusStop         value & 16 == 16   whether the vehicle is stopped at a bus stop
// isAtContainerStop   value & 32 == 32   whether the vehicle is stopped at a container stop
uint8_t TraCI_Commands::vehicleGetStopState(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_STOPSTATE, "vehicleGetStopState");

    uint8_t result = genericGetUnsignedByte(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_STOPSTATE, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_STOPSTATE, "vehicleGetStopState");

    return result;
}


TraCICoord TraCI_Commands::vehicleGetPosition(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_POSITION, "vehicleGetPosition");

    TraCICoord result = genericGetCoord(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_POSITION, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_POSITION, "vehicleGetPosition");

    return result;
}


std::string TraCI_Commands::vehicleGetEdgeID(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_ROAD_ID, "vehicleGetEdgeID");

    std::string result = genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROAD_ID, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_ROAD_ID, "vehicleGetEdgeID");

    return result;
}


std::string TraCI_Commands::vehicleGetLaneID(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_LANE_ID, "vehicleGetLaneID");

    std::string result = genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_ID, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_LANE_ID, "vehicleGetLaneID");

    return result;
}


uint32_t TraCI_Commands::vehicleGetLaneIndex(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_LANE_INDEX, "vehicleGetLaneIndex");

    int32_t result = genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_INDEX, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_LANE_INDEX, "vehicleGetLaneIndex");

    return result;
}


double TraCI_Commands::vehicleGetLanePosition(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_LANEPOSITION, "vehicleGetLanePosition");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANEPOSITION, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_LANEPOSITION, "vehicleGetLanePosition");

    return result;
}


std::string TraCI_Commands::vehicleGetTypeID(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_TYPE, "vehicleGetTypeID");

    std::string result = genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TYPE, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_TYPE, "vehicleGetTypeID");

    return result;
}


std::string TraCI_Commands::vehicleGetRouteID(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_ROUTE_ID, "vehicleGetRouteID");

    std::string result = genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROUTE_ID, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_ROUTE_ID, "vehicleGetRouteID");

    return result;
}


std::vector<std::string> TraCI_Commands::vehicleGetRoute(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_EDGES, "vehicleGetRoute");

    auto result = genericGetStringVector(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_EDGES, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_EDGES, "vehicleGetRoute");

    return result;
}


uint32_t TraCI_Commands::vehicleGetRouteIndex(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_ROUTE_INDEX, "vehicleGetRouteIndex");

    int32_t result = genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROUTE_INDEX, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_ROUTE_INDEX, "vehicleGetRouteIndex");

    return result;
}


double TraCI_Commands::vehicleGetDrivingDistance(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_DISTANCE, "vehicleGetDistance");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_DISTANCE, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_DISTANCE, "vehicleGetDistance");

    return result;
}


std::map<int,bestLanesEntry_t> TraCI_Commands::vehicleGetBestLanes(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_BEST_LANES, "vehicleGetBestLanes");

    uint8_t resultTypeId = TYPE_COMPOUND;
    uint8_t variableId = VAR_BEST_LANES;
    uint8_t responseId = RESPONSE_GET_VEHICLE_VARIABLE;

    TraCIBuffer buf = connection->query(CMD_GET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == nodeId);

    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint32_t count; buf >> count;

    // number of information packets (in 'No' variable)
    uint8_t typeI; buf >> typeI;
    uint32_t No; buf >> No;

    std::map<int,bestLanesEntry_t> final;

    for (uint32_t i = 0; i < No; ++i)
    {
        bestLanesEntry_t entry = {};
        entry.length = -1;
        entry.occupation = -1;
        entry.offset = -1;
        entry.continuingDrive = -1;
        entry.best.clear();

        // get lane id
        int8_t sType; buf >> sType;
        std::string laneId; buf >> laneId;
        entry.laneId = laneId;

        // length
        int8_t dType1; buf >> dType1;
        double length; buf >> length;
        entry.length = length;

        // occupation
        int8_t dType2; buf >> dType2;
        double occupation; buf >> occupation;
        entry.occupation = occupation;

        // offset
        int8_t bType; buf >> bType;
        int8_t offset; buf >> offset;
        entry.offset = offset;

        // continuing drive?
        int8_t bType2; buf >> bType2;
        uint8_t continuingDrive; buf >> continuingDrive;
        entry.continuingDrive = continuingDrive;

        // best subsequent lanes
        std::vector<std::string> res;
        int8_t bType3; buf >> bType3;
        uint32_t count; buf >> count;
        for (uint32_t i = 0; i < count; i++)
        {
            std::string id; buf >> id;
            res.push_back(id);
        }
        entry.best = res;

        // add into the map
        final.insert( std::make_pair(i,entry) );
    }

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_BEST_LANES, "vehicleGetBestLanes");

    return final;
}


colorVal_t TraCI_Commands::vehicleGetColor(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_COLOR, "vehicleGetColor");

    uint8_t variableId = VAR_COLOR;
    TraCIBuffer buf = connection->query(CMD_GET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == RESPONSE_GET_VEHICLE_VARIABLE);
    uint8_t varId; buf >> varId;
    ASSERT(varId == VAR_COLOR);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == nodeId);
    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == TYPE_COLOR);

    colorVal_t entry = {};

    // now we start getting real data that we are looking for
    buf >> entry.red;
    buf >> entry.green;
    buf >> entry.blue;
    buf >> entry.alpha;

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_COLOR, "vehicleGetColor");

    return entry;
}


VehicleSignal_t TraCI_Commands::vehicleGetSignalStatus(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_SIGNALS, "vehicleGetSignalStatus");

    uint32_t result = genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x5b, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_SIGNALS, "vehicleGetSignalStatus");

    return (VehicleSignal_t) result;
}


double TraCI_Commands::vehicleGetLength(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_LENGTH, "vehicleGetLength");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LENGTH, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_LENGTH, "vehicleGetLength");

    return result;
}


double TraCI_Commands::vehicleGetMinGap(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_MINGAP, "vehicleGetMinGap");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_MINGAP, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_MINGAP, "vehicleGetMinGap");

    return result;
}

double TraCI_Commands::vehicleGetWaitingTime(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_WAITING_TIME, "vehicleGetWaitingTime");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_WAITING_TIME, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_WAITING_TIME, "vehicleGetWaitingTime");

    return result;
}

double TraCI_Commands::vehicleGetMaxSpeed(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_MAXSPEED, "vehicleGetMaxSpeed");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_MAXSPEED, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_MAXSPEED, "vehicleGetMaxSpeed");

    return result;
}


double TraCI_Commands::vehicleGetMaxAccel(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_ACCEL, "vehicleGetMaxAccel");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ACCEL, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_ACCEL, "vehicleGetMaxAccel");

    return result;
}


double TraCI_Commands::vehicleGetMaxDecel(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_DECEL, "vehicleGetMaxDecel");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_DECEL, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_DECEL, "vehicleGetMaxDecel");

    return result;
}


double TraCI_Commands::vehicleGetTimeGap(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_TAU, "vehicleGetTimeGap");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TAU, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_TAU, "vehicleGetTimeGap");

    return result;
}


std::string TraCI_Commands::vehicleGetClass(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_VEHICLECLASS, "vehicleGetClass");

    std::string result = genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_VEHICLECLASS, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_VEHICLECLASS, "vehicleGetClass");

    return result;
}


leader_t TraCI_Commands::vehicleGetLeader(std::string nodeId, double look_ahead_distance)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_LEADER, "vehicleGetLeader");

    uint8_t requestTypeId = TYPE_DOUBLE;
    uint8_t resultTypeId = TYPE_COMPOUND;
    uint8_t variableId = VAR_LEADER;
    uint8_t responseId = RESPONSE_GET_VEHICLE_VARIABLE;

    TraCIBuffer buf = connection->query(CMD_GET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << requestTypeId << look_ahead_distance);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == nodeId);

    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint32_t count; buf >> count;

    // now we start getting real data that we are looking for
    leader_t entry = {};

    uint8_t dType; buf >> dType;
    std::string id; buf >> id;
    entry.leaderID = id;

    uint8_t dType2; buf >> dType2;
    double len; buf >> len;
    // the distance does not include minGap. we will add minGap to the len
    len += vehicleGetMinGap(nodeId);
    entry.distance2Leader = len;

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_LEADER, "vehicleGetLeader");

    return entry;
}


std::vector<TL_info_t> TraCI_Commands::vehicleGetNextTLS(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_NEXT_TLS, "vehicleGetNextTLS");

    uint8_t resultTypeId = TYPE_COMPOUND;
    uint8_t variableId = VAR_NEXT_TLS;
    uint8_t responseId = RESPONSE_GET_VEHICLE_VARIABLE;

    TraCIBuffer buf = connection->query(CMD_GET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == nodeId);

    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint32_t count; buf >> count;

    // now we start getting real data that we are looking for
    std::vector<TL_info_t> res;

    uint8_t typeI; buf >> typeI;
    uint32_t NoTLs; buf >> NoTLs;

    for(uint32_t i = 0; i < NoTLs; ++i)
    {
        TL_info_t entry = {};

        uint8_t typeI2; buf >> typeI2;
        std::string TLS_id; buf >> TLS_id;
        entry.TLS_id = TLS_id;

        uint8_t typeI3; buf >> typeI3;
        uint32_t TLS_link_index; buf >> TLS_link_index;
        entry.TLS_link_index = TLS_link_index;

        uint8_t typeI4; buf >> typeI4;
        double TLS_distance; buf >> TLS_distance;
        entry.TLS_distance = TLS_distance;

        uint8_t typeI5; buf >> typeI5;
        uint8_t linkState; buf >> linkState;
        entry.linkState = linkState;

        res.push_back(entry);
    }

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_NEXT_TLS, "vehicleGetNextTLS");

    return res;
}


double TraCI_Commands::vehicleGetCO2Emission(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_CO2EMISSION, "vehicleGetCO2Emission");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_CO2EMISSION, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_CO2EMISSION, "vehicleGetCO2Emission");

    return result;
}


double TraCI_Commands::vehicleGetCOEmission(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_COEMISSION, "vehicleGetCOEmission");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_COEMISSION, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_COEMISSION, "vehicleGetCOEmission");

    return result;
}


double TraCI_Commands::vehicleGetHCEmission(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_HCEMISSION, "vehicleGetHCEmission");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_HCEMISSION, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_HCEMISSION, "vehicleGetHCEmission");

    return result;
}


double TraCI_Commands::vehicleGetPMxEmission(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_PMXEMISSION, "vehicleGetPMxEmission");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_PMXEMISSION, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_PMXEMISSION, "vehicleGetPMxEmission");

    return result;
}


double TraCI_Commands::vehicleGetNOxEmission(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_NOXEMISSION, "vehicleGetNOxEmission");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_NOXEMISSION, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_NOXEMISSION, "vehicleGetNOxEmission");

    return result;
}


double TraCI_Commands::vehicleGetNoiseEmission(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_NOISEEMISSION, "vehicleGetNoiseEmission");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_NOISEEMISSION, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_NOISEEMISSION, "vehicleGetNoiseEmission");

    return result;
}


double TraCI_Commands::vehicleGetFuelConsumption(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_FUELCONSUMPTION, "vehicleGetFuelConsumption");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_FUELCONSUMPTION, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_FUELCONSUMPTION, "vehicleGetFuelConsumption");

    return result;
}


std::string TraCI_Commands::vehicleGetEmissionClass(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_EMISSIONCLASS, "vehicleGetEmissionClass");

    std::string result = genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_EMISSIONCLASS, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_EMISSIONCLASS, "vehicleGetEmissionClass");

    return result;
}


double TraCI_Commands::vehicleGetCurrentAccel(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, 0x74, "vehicleGetCurrentAccel");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x74, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, 0x74, "vehicleGetCurrentAccel");

    return result;
}


double TraCI_Commands::vehicleGetDepartureTime(std::string nodeId)
{
    auto it = departureArrival.find(nodeId);

    if(it == departureArrival.end())
        return -1;
    else
        return it->second.departure;
}


double TraCI_Commands::vehicleGetArrivalTime(std::string nodeId)
{
    auto it = departureArrival.find(nodeId);

    if(it == departureArrival.end())
        return -1;
    else
        return it->second.arrival;
}


std::string TraCI_Commands::vehicleGetCarFollowingModelName(std::string nodeId)
{
    carFollowingModel_t number = vehicleGetCarFollowingModelID(nodeId);

    static std::map<carFollowingModel_t, std::string> carFollowingModelName = {
            {SUMO_CF_KRAUSS, "KRAUSS"},
            {SUMO_CF_KRAUSS_PLUS_SLOPE, "KRAUSS_PLUS_SLOPE"},
            {SUMO_CF_KRAUSS_ORIG1, "KRAUSS_ORIG1"},
            {SUMO_CF_SMART_SK, "SMART_SK"},
            {SUMO_CF_DANIEL1, "DANIEL1"},
            {SUMO_CF_IDM, "IDM"},
            {SUMO_CF_IDMM, "IDMM"},
            {SUMO_CF_PWAGNER2009, "PWAGNER2009"},
            {SUMO_CF_BKERNER, "BKERNER"},
            {SUMO_CF_WIEDEMANN, "WIEDEMANN"},
            {SUMO_CF_OPTIMALSPEED, "OPTIMALSPEED"},
            {SUMO_CF_KRAUSSFIXED, "KRAUSS_FIXED"},
            {SUMO_CF_ACC, "ACC"},
            {SUMO_CF_CACC, "CACC"}
    };

    auto it = carFollowingModelName.find(number);
    if(it == carFollowingModelName.end())
        throw omnetpp::cRuntimeError("Invalid car-following model number %d", (int)number);

    return it->second;
}


carFollowingModel_t TraCI_Commands::vehicleGetCarFollowingModelID(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, 0x72, "vehicleGetCarFollowingModelID");

    int result = genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x72, RESPONSE_GET_VEHICLE_VARIABLE);

    if(result < 0 || result >= (int)SUMO_CF_MAX)
        throw omnetpp::cRuntimeError("Invalid car-following model number %d", result);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, 0x72, "vehicleGetCarFollowingModelID");

    return (carFollowingModel_t)result;
}


int TraCI_Commands::vehicleGetCACCStrategy(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, 0x73, "vehicleGetCACCStrategy");

    int result = genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x73, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, 0x73, "vehicleGetCACCStrategy");

    return result;
}


CFMODES_t TraCI_Commands::vehicleGetCarFollowingModelMode(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, 0x75, "vehicleGetCarFollowingModelMode");

    int result = genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x75, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, 0x75, "vehicleGetCarFollowingModelMode");

    return (CFMODES_t) result;
}


std::string TraCI_Commands::vehicleGetPlatoonId(std::string nodeId)
{
    std::string omnetId = convertId_traci2omnet(nodeId);
    if(omnetId == "")
        throw omnetpp::cRuntimeError("Vehicle with OMNET++ id '%s' does not exists", nodeId.c_str());

    cModule *module = omnetpp::getSimulation()->getSystemModule()->getModuleByPath(omnetId.c_str());
    ASSERT(module);

    cModule *appl = module->getSubmodule("appl");
    ASSERT(appl);

    // get a pointer to the application layer
    ApplVManager *vehPtr = static_cast<ApplVManager *>(appl);
    ASSERT(vehPtr);

    return vehPtr->par("myPlnID").stringValue();
}


int TraCI_Commands::vehicleGetPlatoonDepth(std::string nodeId)
{
    std::string omnetId = convertId_traci2omnet(nodeId);
    if(omnetId == "")
        throw omnetpp::cRuntimeError("Vehicle with OMNET++ id '%s' does not exists", nodeId.c_str());

    cModule *module = omnetpp::getSimulation()->getSystemModule()->getModuleByPath(omnetId.c_str());
    ASSERT(module);

    cModule *appl = module->getSubmodule("appl");
    ASSERT(appl);

    // get a pointer to the application layer
    ApplVManager *vehPtr = static_cast<ApplVManager *>(appl);
    ASSERT(vehPtr);

    return vehPtr->par("myPlnDepth").intValue();
}


bool TraCI_Commands::vehicleExist(std::string nodeId)
{
    auto it = hosts.find(nodeId);

    if(it == hosts.end())
        return false;
    else
        return true;
}


// #########################
// CMD_SET_VEHICLE_VARIABLE
// #########################

// adds or modifies a stop with the given parameters
// Let the vehicle stop at the given edge, at the given position and lane.
// The vehicle will stop for the given duration.
void TraCI_Commands::vehicleSetStop(std::string nodeId, std::string edgeId, double stopPos, uint8_t laneId, int32_t duration, uint8_t flag)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, CMD_STOP, "vehicleSetStop");

    uint8_t variableId = CMD_STOP;
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 5;
    uint8_t edgeIdT = TYPE_STRING;
    uint8_t stopPosT = TYPE_DOUBLE;
    uint8_t stopLaneT = TYPE_BYTE;
    uint8_t durationT = TYPE_INTEGER;
    uint8_t flagT = TYPE_BYTE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId
            << variableType << count
            << edgeIdT << edgeId
            << stopPosT << stopPos
            << stopLaneT << laneId
            << durationT << duration
            << flagT << flag);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, CMD_STOP, "vehicleSetStop");
}


void TraCI_Commands::vehicleResume(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, CMD_RESUME, "vehicleResume");

    uint8_t variableId = CMD_RESUME;
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 0;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId
            << variableType << count);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, CMD_RESUME, "vehicleResume");
}


void TraCI_Commands::vehicleSetSpeed(std::string nodeId, double speed)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, VAR_SPEED, "vehicleSetSpeed");

    uint8_t variableId = VAR_SPEED;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, VAR_SPEED, "vehicleSetSpeed");
}


void TraCI_Commands::vehicleSetSpeedMode(std::string nodeId, uint32_t bitset)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, VAR_SPEEDSETMODE, "vehicleSetSpeedMode");

    uint8_t variableId = VAR_SPEEDSETMODE;
    uint8_t variableType = TYPE_INTEGER;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << bitset);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, VAR_SPEEDSETMODE, "vehicleSetSpeedMode");
}


int32_t TraCI_Commands::vehicleBuildLaneChangeMode(uint8_t TraciLaneChangePriority, uint8_t RightDriveLC, uint8_t SpeedGainLC, uint8_t CooperativeLC, uint8_t StrategicLC)
{
    // only two less-significant bits are needed
    StrategicLC = StrategicLC & 3;
    CooperativeLC = CooperativeLC & 3;
    SpeedGainLC = SpeedGainLC & 3;
    RightDriveLC = RightDriveLC & 3;
    TraciLaneChangePriority = TraciLaneChangePriority & 3;

    int32_t bitset = StrategicLC + (CooperativeLC << 2) + (SpeedGainLC << 4) + (RightDriveLC << 6) + (TraciLaneChangePriority << 8);

    return bitset;
}


void TraCI_Commands::vehicleSetLaneChangeMode(std::string nodeId, int32_t bitset)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, VAR_LANECHANGE_MODE, "vehicleSetLaneChangeMode");

    uint8_t variableId = 0xb6;
    uint8_t variableType = TYPE_INTEGER;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << bitset);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, VAR_LANECHANGE_MODE, "vehicleSetLaneChangeMode");
}


void TraCI_Commands::vehicleChangeLane(std::string nodeId, uint8_t laneId, double duration)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, CMD_CHANGELANE, "vehicleChangeLane");

    uint8_t variableId = CMD_CHANGELANE;
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 2;
    uint8_t laneIdT = TYPE_BYTE;
    uint8_t durationT = TYPE_INTEGER;
    uint32_t durationMS = duration * 1000;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId
            << variableType << count
            << laneIdT << laneId
            << durationT << durationMS);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, CMD_CHANGELANE, "vehicleChangeLane");
}


void TraCI_Commands::vehicleSetRoute(std::string id, std::list<std::string> value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, VAR_ROUTE, "vehicleSetRoute");

    uint8_t variableId = VAR_ROUTE;
    uint8_t variableTypeSList = TYPE_STRINGLIST;

    TraCIBuffer buffer;
    buffer << variableId << id << variableTypeSList << (int32_t)value.size();
    for(auto &str : value)
    {
        buffer << (int32_t)str.length();
        for(unsigned int i = 0; i < str.length(); ++i)
            buffer << (int8_t)str[i];
    }
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, buffer);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, VAR_ROUTE, "vehicleSetRoute");
}


void TraCI_Commands::vehicleSetRouteID(std::string nodeId, std::string routeID)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, VAR_ROUTE_ID, "vehicleSetRouteID");

    uint8_t variableId = VAR_ROUTE_ID;
    uint8_t variableType = TYPE_STRING;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << routeID);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, VAR_ROUTE_ID, "vehicleSetRouteID");
}


void TraCI_Commands::vehicleChangeTarget(std::string nodeId, std::string destEdgeID)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, CMD_CHANGETARGET, "vehicleChangeTarget");

    uint8_t variableId = CMD_CHANGETARGET;
    uint8_t variableType = TYPE_STRING;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << destEdgeID);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, CMD_CHANGETARGET, "vehicleChangeTarget");
}


void TraCI_Commands::vehicleMoveTo(std::string nodeId, std::string laneId, double pos)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, VAR_MOVE_TO, "vehicleMoveTo");

    uint8_t variableId = VAR_MOVE_TO;
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 2;
    uint8_t laneIdT = TYPE_STRING;
    uint8_t posT = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId
            << variableType << count
            << laneIdT << laneId
            << posT << pos);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, VAR_MOVE_TO, "vehicleMoveTo");
}


void TraCI_Commands::vehicleSetColor(std::string nodeId, const colorVal_t color)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, VAR_COLOR, "vehicleSetColor");

    TraCIBuffer p;
    p << static_cast<uint8_t>(VAR_COLOR);
    p << nodeId;
    p << static_cast<uint8_t>(TYPE_COLOR) << (uint8_t)color.red << (uint8_t)color.green << (uint8_t)color.blue << (uint8_t)color.alpha;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, p);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, VAR_COLOR, "vehicleSetColor");
}


void TraCI_Commands::vehicleSetColor(std::string nodeId, const RGB color)
{
    colorVal_t color_sumo;

    color_sumo.red = color.red;
    color_sumo.green = color.green;
    color_sumo.blue = color.blue;
    color_sumo.alpha = 255;

    vehicleSetColor(nodeId, color_sumo);
}


void TraCI_Commands::vehicleSetClass(std::string nodeId, std::string vClass)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, VAR_VEHICLECLASS, "vehicleSetClass");

    uint8_t variableId = VAR_VEHICLECLASS;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << vClass);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, VAR_VEHICLECLASS, "vehicleSetClass");
}


void TraCI_Commands::vehicleSetLength(std::string nodeId, double value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, VAR_LENGTH, "vehicleSetLength");

    uint8_t variableId = VAR_LENGTH;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, VAR_LENGTH, "vehicleSetLength");
}


void TraCI_Commands::vehicleSetWidth(std::string nodeId, double value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, VAR_WIDTH, "vehicleSetWidth");

    uint8_t variableId = VAR_WIDTH;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, VAR_WIDTH, "vehicleSetWidth");
}

void TraCI_Commands::vehicleSlowDown(std::string nodeId, double speed, int duration)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, CMD_SLOWDOWN, "vehicleSlowDown");

    uint8_t variableId = CMD_SLOWDOWN;

    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 2;

    uint8_t speedT = TYPE_DOUBLE;

    uint8_t durationT = TYPE_INTEGER;
    uint32_t durationMS = duration * 1000;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() 
            << variableId << nodeId
            << variableType << count
            << speedT << speed
            << durationT << durationMS);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, CMD_SLOWDOWN, "vehicleSlowDown");
}

void TraCI_Commands::vehicleSetSignalStatus(std::string nodeId, int32_t bitset)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, VAR_SIGNALS, "vehicleSetSignalStatus");

    uint8_t variableId = VAR_SIGNALS;
    uint8_t variableType = TYPE_INTEGER;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << bitset);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, VAR_SIGNALS, "vehicleSetSignalStatus");
}


void TraCI_Commands::vehicleSetMaxSpeed(std::string nodeId, double value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, VAR_MAXSPEED, "vehicleSetMaxSpeed");

    uint8_t variableId = VAR_MAXSPEED;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, VAR_MAXSPEED, "vehicleSetMaxSpeed");
}


void TraCI_Commands::vehicleSetMaxAccel(std::string nodeId, double value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, VAR_ACCEL, "vehicleSetMaxAccel");

    uint8_t variableId = VAR_ACCEL;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, VAR_ACCEL, "vehicleSetMaxAccel");
}


void TraCI_Commands::vehicleSetMaxDecel(std::string nodeId, double value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, VAR_DECEL, "vehicleSetMaxDecel");

    uint8_t variableId = VAR_DECEL;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, VAR_DECEL, "vehicleSetMaxDecel");
}


// this changes vehicle type!
void TraCI_Commands::vehicleSetTimeGap(std::string nodeId, double value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, VAR_TAU, "vehicleSetTimeGap");

    uint8_t variableId = VAR_TAU;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, VAR_TAU, "vehicleSetTimeGap");
}


void TraCI_Commands::vehicleAdd(std::string vehicleId, std::string vehicleTypeId, std::string routeId, int32_t depart, double pos, double speed, uint8_t lane)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, ADD, "vehicleAdd");

    uint8_t variableId = ADD;
    uint8_t variableType = TYPE_COMPOUND;
    uint8_t variableTypeS = TYPE_STRING;
    uint8_t variableTypeI = TYPE_INTEGER;
    uint8_t variableTypeD = TYPE_DOUBLE;
    uint8_t variableTypeB = TYPE_BYTE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << vehicleId
            << variableType << (int32_t) 6
            << variableTypeS
            << vehicleTypeId
            << variableTypeS
            << routeId
            << variableTypeI
            << depart        // departure time
            << variableTypeD
            << pos           // departure position
            << variableTypeD
            << speed         // departure speed
            << variableTypeB
            << lane);        // departure lane

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, ADD, "vehicleAdd");
}


void TraCI_Commands::vehicleRemove(std::string nodeId, uint8_t reason)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, REMOVE, "vehicleRemove");

    uint8_t variableId = REMOVE;
    uint8_t variableType = TYPE_BYTE;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << reason);

    ASSERT(buf.eof());

    // unsubscribe the removed vehicle
    std::vector<uint8_t> variables;
    buf = subscribeVehicle(0, 0x7FFFFFFF, nodeId, variables);
    ASSERT(buf.eof());

    removed_vehicles.push_back(nodeId);

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, REMOVE, "vehicleRemove");
}


void TraCI_Commands::vehiclePlatoonInit(std::string nodeId, std::deque<std::string> platoonMembers)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, 0x26, "vehiclePlatoonInit");

    uint8_t variableId = 0x26;
    uint8_t variableType = TYPE_STRINGLIST;

    TraCIBuffer buffer;
    buffer << variableId << nodeId << variableType << (int32_t)platoonMembers.size();
    for(auto &str : platoonMembers)
    {
        buffer << (int32_t)str.length();
        for(unsigned int i = 0; i < str.length(); ++i)
            buffer << (int8_t)str[i];
    }

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, buffer);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, 0x26, "vehiclePlatoonInit");
}


void TraCI_Commands::vehiclePlatoonViewUpdate(std::string nodeId, std::string value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, 0x15, "vehiclePlatoonViewUpdate");

    uint8_t variableId = 0x15;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, 0x15, "vehiclePlatoonViewUpdate");
}


void TraCI_Commands::vehicleSetErrorGap(std::string nodeId, double value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, 0x20, "vehicleSetErrorGap");

    uint8_t variableId = 0x20;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, 0x20, "vehicleSetErrorGap");
}


void TraCI_Commands::vehicleSetErrorRelSpeed(std::string nodeId, double value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, 0x21, "vehicleSetErrorRelSpeed");

    uint8_t variableId = 0x21;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, 0x21, "vehicleSetErrorRelSpeed");
}


void TraCI_Commands::vehicleSetDebug(std::string nodeId, bool value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, 0x16, "vehicleSetDebug");

    uint8_t variableId = 0x16;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << (int)value);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, 0x16, "vehicleSetDebug");
}


void TraCI_Commands::vehicleSetVint(std::string nodeId, double value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, 0x22, "vehicleSetVint");

    uint8_t variableId = 0x22;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, 0x22, "vehicleSetVint");
}


void TraCI_Commands::vehicleSetComfAccel(std::string nodeId, double speed)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, 0x23, "vehicleSetComfAccel");

    uint8_t variableId = 0x23;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, 0x23, "vehicleSetComfAccel");
}


void TraCI_Commands::vehicleSetComfDecel(std::string nodeId, double speed)
{
    record_TraCI_activity_func(commandStart, CMD_SET_VEHICLE_VARIABLE, 0x24, "vehicleSetComfDecel");

    uint8_t variableId = 0x24;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_VEHICLE_VARIABLE, 0x24, "vehicleSetComfDecel");
}


// ################################################################
//                          vehicle type
// ################################################################

// ############################
// CMD_GET_VEHICLETYPE_VARIABLE
// ############################

std::vector<std::string> TraCI_Commands::vehicleTypeGetIDList()
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLETYPE_VARIABLE, ID_LIST, "vehicleTypeGetIDList");

    auto result = genericGetStringVector(CMD_GET_VEHICLETYPE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLETYPE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLETYPE_VARIABLE, ID_LIST, "vehicleTypeGetIDList");

    return result;
}


uint32_t TraCI_Commands::vehicleTypeGetIDCount()
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLETYPE_VARIABLE, ID_COUNT, "vehicleTypeGetIDCount");

    uint32_t result = genericGetInt(CMD_GET_VEHICLETYPE_VARIABLE, "", ID_COUNT, RESPONSE_GET_VEHICLETYPE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLETYPE_VARIABLE, ID_COUNT, "vehicleTypeGetIDCount");

    return result;
}


double TraCI_Commands::vehicleTypeGetLength(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLETYPE_VARIABLE, VAR_LENGTH, "vehicleTypeGetLength");

    double result = genericGetDouble(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, VAR_LENGTH, RESPONSE_GET_VEHICLETYPE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLETYPE_VARIABLE, VAR_LENGTH, "vehicleTypeGetLength");

    return result;
}


double TraCI_Commands::vehicleTypeGetMaxSpeed(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLETYPE_VARIABLE, VAR_MAXSPEED, "vehicleTypeGetMaxSpeed");

    double result = genericGetDouble(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, VAR_MAXSPEED, RESPONSE_GET_VEHICLETYPE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLETYPE_VARIABLE, VAR_MAXSPEED, "vehicleTypeGetMaxSpeed");

    return result;
}


double TraCI_Commands::vehicleTypeGetMinGap(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLETYPE_VARIABLE, VAR_MINGAP, "vehicleTypeGetMinGap");

    double result = genericGetDouble(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, VAR_MINGAP, RESPONSE_GET_VEHICLETYPE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLETYPE_VARIABLE, VAR_MINGAP, "vehicleTypeGetMinGap");

    return result;
}


// ################################################################
//                              route
// ################################################################

// ######################
// CMD_GET_ROUTE_VARIABLE
// ######################

std::vector<std::string> TraCI_Commands::routeGetIDList()
{
    record_TraCI_activity_func(commandStart, CMD_GET_ROUTE_VARIABLE, ID_LIST, "routeGetIDList");

    auto result = genericGetStringVector(CMD_GET_ROUTE_VARIABLE, "", ID_LIST, RESPONSE_GET_ROUTE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_ROUTE_VARIABLE, ID_LIST, "routeGetIDList");

    return result;
}


uint32_t TraCI_Commands::routeGetIDCount()
{
    record_TraCI_activity_func(commandStart, CMD_GET_ROUTE_VARIABLE, ID_COUNT, "routeGetIDCount");

    uint32_t result = genericGetInt(CMD_GET_ROUTE_VARIABLE, "", ID_COUNT, RESPONSE_GET_ROUTE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_ROUTE_VARIABLE, ID_COUNT, "routeGetIDCount");

    return result;
}


std::vector<std::string> TraCI_Commands::routeGetEdges(std::string routeID)
{
    record_TraCI_activity_func(commandStart, CMD_GET_ROUTE_VARIABLE, VAR_EDGES, "routeGetEdges");

    auto result = genericGetStringVector(CMD_GET_ROUTE_VARIABLE, routeID, VAR_EDGES, RESPONSE_GET_ROUTE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_ROUTE_VARIABLE, VAR_EDGES, "routeGetEdges");

    return result;
}


// todo: calculate the shortest route between 'from_edge' and 'to_edge'
std::vector<std::string> TraCI_Commands::routeShortest(std::string from_edge, std::string to_edge)
{
    static int counter = 0;
    std::string route_name = "newRoute" + std::to_string(counter);
    std::string veh_name = "new_veh" + std::to_string(counter);

    // create a new route that consists of 'from_edge'
    std::vector<std::string> newRoute = {from_edge};
    routeAdd(route_name, newRoute);

    // add a vehicle with that route
    vehicleAdd(veh_name, "DEFAULT_VEHTYPE", route_name, 0, 0, 0, -4 /*lane*/);

    // compute a new route to the destination
    vehicleChangeTarget(veh_name, to_edge);

    // retrieve the edges of the new route
    auto route = vehicleGetRoute(veh_name);

    vehicleRemove(veh_name, 2);

    counter++;

    return route;
}


// #######################
// CMD_SET_ROUTE_VARIABLE
// #######################

void TraCI_Commands::routeAdd(std::string name, std::vector<std::string> route)
{
    record_TraCI_activity_func(commandStart, CMD_SET_ROUTE_VARIABLE, ADD, "routeAdd");

    uint8_t variableId = ADD;
    uint8_t variableTypeS = TYPE_STRINGLIST;

    TraCIBuffer buffer;
    buffer << variableId << name << variableTypeS << (int32_t)route.size();
    for(auto &str : route)
    {
        buffer << (int32_t)str.length();
        for(unsigned int i = 0; i < str.length(); ++i)
            buffer << (int8_t)str[i];
    }

    TraCIBuffer buf = connection->query(CMD_SET_ROUTE_VARIABLE, buffer);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_ROUTE_VARIABLE, ADD, "routeAdd");
}


// ################################################################
//                              edge
// ################################################################

// ######################
// CMD_GET_EDGE_VARIABLE
// ######################

std::vector<std::string> TraCI_Commands::edgeGetIDList()
{
    record_TraCI_activity_func(commandStart, CMD_GET_EDGE_VARIABLE, ID_LIST, "edgeGetIDList");

    auto result = genericGetStringVector(CMD_GET_EDGE_VARIABLE, "", ID_LIST, RESPONSE_GET_EDGE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_EDGE_VARIABLE, ID_LIST, "edgeGetIDList");

    return result;
}


uint32_t TraCI_Commands::edgeGetIDCount()
{
    record_TraCI_activity_func(commandStart, CMD_GET_EDGE_VARIABLE, ID_COUNT, "edgeGetIDCount");

    uint32_t result = genericGetInt(CMD_GET_EDGE_VARIABLE, "", ID_COUNT, RESPONSE_GET_EDGE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_EDGE_VARIABLE, ID_COUNT, "edgeGetIDCount");

    return result;
}


double TraCI_Commands::edgeGetMeanTravelTime(std::string Id)
{
    record_TraCI_activity_func(commandStart, CMD_GET_EDGE_VARIABLE, VAR_CURRENT_TRAVELTIME, "edgeGetMeanTravelTime");

    double result = genericGetDouble(CMD_GET_EDGE_VARIABLE, Id, VAR_CURRENT_TRAVELTIME, RESPONSE_GET_EDGE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_EDGE_VARIABLE, VAR_CURRENT_TRAVELTIME, "edgeGetMeanTravelTime");

    return result;
}


uint32_t TraCI_Commands::edgeGetLastStepVehicleNumber(std::string Id)
{
    record_TraCI_activity_func(commandStart, CMD_GET_EDGE_VARIABLE, LAST_STEP_VEHICLE_NUMBER, "edgeGetLastStepVehicleNumber");

    uint32_t result = genericGetInt(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_VEHICLE_NUMBER, RESPONSE_GET_EDGE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_EDGE_VARIABLE, LAST_STEP_VEHICLE_NUMBER, "edgeGetLastStepVehicleNumber");

    return result;
}


std::vector<std::string> TraCI_Commands::edgeGetLastStepVehicleIDs(std::string Id)
{
    record_TraCI_activity_func(commandStart, CMD_GET_EDGE_VARIABLE, LAST_STEP_VEHICLE_ID_LIST, "edgeGetLastStepVehicleIDs");

    auto result = genericGetStringVector(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_EDGE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_EDGE_VARIABLE, LAST_STEP_VEHICLE_ID_LIST, "edgeGetLastStepVehicleIDs");

    return result;
}


double TraCI_Commands::edgeGetLastStepMeanVehicleSpeed(std::string Id)
{
    record_TraCI_activity_func(commandStart, CMD_GET_EDGE_VARIABLE, LAST_STEP_MEAN_SPEED, "edgeGetLastStepMeanVehicleSpeed");

    double result = genericGetDouble(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_MEAN_SPEED, RESPONSE_GET_EDGE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_EDGE_VARIABLE, LAST_STEP_MEAN_SPEED, "edgeGetLastStepMeanVehicleSpeed");

    return result;
}


double TraCI_Commands::edgeGetLastStepMeanVehicleLength(std::string Id)
{
    record_TraCI_activity_func(commandStart, CMD_GET_EDGE_VARIABLE, LAST_STEP_LENGTH, "edgeGetLastStepMeanVehicleLength");

    double result = genericGetDouble(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_LENGTH, RESPONSE_GET_EDGE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_EDGE_VARIABLE, LAST_STEP_LENGTH, "edgeGetLastStepMeanVehicleLength");

    return result;
}


std::vector<std::string> TraCI_Commands::edgeGetLastStepPersonIDs(std::string Id)
{
    record_TraCI_activity_func(commandStart, CMD_GET_EDGE_VARIABLE, LAST_STEP_PERSON_ID_LIST, "edgeGetLastStepPersonIDs");

    auto result = genericGetStringVector(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_PERSON_ID_LIST, RESPONSE_GET_EDGE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_EDGE_VARIABLE, LAST_STEP_PERSON_ID_LIST, "edgeGetLastStepPersonIDs");

    return result;
}


uint32_t TraCI_Commands::edgeGetLaneCount(std::string Id)
{
    record_TraCI_activity_func(commandStart, CMD_GET_EDGE_VARIABLE, 0x03, "edgeGetLaneCount");

    uint32_t result = genericGetInt(CMD_GET_EDGE_VARIABLE, Id, 0x03, RESPONSE_GET_EDGE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_EDGE_VARIABLE, 0x03, "edgeGetLaneCount");

    return result;
}


std::vector<std::string> TraCI_Commands::edgeGetAllowedLanes(std::string Id, std::string vClass)
{
    record_TraCI_activity_func(commandStart, CMD_GET_EDGE_VARIABLE, 0x04, "edgeGetAllowedLanes");

    uint8_t requestTypeId = TYPE_STRING;
    uint8_t resultTypeId = TYPE_STRINGLIST;
    uint8_t variableId = 0x04;
    uint8_t responseId = RESPONSE_GET_EDGE_VARIABLE;

    TraCIBuffer buf = connection->query(CMD_GET_EDGE_VARIABLE, TraCIBuffer() << variableId << Id << requestTypeId << vClass);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == Id);

    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint32_t count; buf >> count;

    std::vector<std::string> res;

    for (uint32_t i = 0; i < count; i++) {
        std::string id; buf >> id;
        res.push_back(id);
    }

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_EDGE_VARIABLE, 0x04, "edgeGetAllowedLanes");

    return res;
}


// ######################
// CMD_SET_EDGE_VARIABLE
// ######################

void TraCI_Commands::edgeSetGlobalTravelTime(std::string edgeId, int32_t beginT, int32_t endT, double value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_EDGE_VARIABLE, VAR_EDGE_TRAVELTIME, "edgeSetGlobalTravelTime");

    uint8_t variableId = VAR_EDGE_TRAVELTIME;
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 3;
    uint8_t valueI = TYPE_INTEGER;
    uint8_t valueD = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_EDGE_VARIABLE, TraCIBuffer() << variableId << edgeId
            << variableType << count
            << valueI << beginT
            << valueI << endT
            << valueD << value);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_EDGE_VARIABLE, VAR_EDGE_TRAVELTIME, "edgeSetGlobalTravelTime");
}


// ################################################################
//                              lane
// ################################################################

// #####################
// CMD_GET_LANE_VARIABLE
// #####################

// gets a list of all lanes in the network
std::vector<std::string> TraCI_Commands::laneGetIDList()
{
    record_TraCI_activity_func(commandStart, CMD_GET_LANE_VARIABLE, ID_LIST, "laneGetIDList");

    auto result = genericGetStringVector(CMD_GET_LANE_VARIABLE, "", ID_LIST, RESPONSE_GET_LANE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_LANE_VARIABLE, ID_LIST, "laneGetIDList");

    return result;
}


uint32_t TraCI_Commands::laneGetIDCount()
{
    record_TraCI_activity_func(commandStart, CMD_GET_LANE_VARIABLE, ID_COUNT, "laneGetIDCount");

    uint32_t result = genericGetInt(CMD_GET_LANE_VARIABLE, "", ID_COUNT, RESPONSE_GET_LANE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_LANE_VARIABLE, ID_COUNT, "laneGetIDCount");

    return result;
}


uint8_t TraCI_Commands::laneGetLinkNumber(std::string laneId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_LANE_VARIABLE, LANE_LINK_NUMBER, "laneGetLinkNumber");

    uint8_t result = genericGetUnsignedByte(CMD_GET_LANE_VARIABLE, laneId, LANE_LINK_NUMBER, RESPONSE_GET_LANE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_LANE_VARIABLE, LANE_LINK_NUMBER, "laneGetLinkNumber");

    return result;
}


std::map<int,linkEntry_t> TraCI_Commands::laneGetLinks(std::string laneId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_LANE_VARIABLE, LANE_LINKS, "laneGetLinks");

    uint8_t resultTypeId = TYPE_COMPOUND;
    uint8_t variableId = LANE_LINKS;
    uint8_t responseId = RESPONSE_GET_LANE_VARIABLE;

    TraCIBuffer buf = connection->query(CMD_GET_LANE_VARIABLE, TraCIBuffer() << variableId << laneId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == laneId);

    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint32_t count; buf >> count;

    // number of information packets (in 'No' variable)
    uint8_t typeI; buf >> typeI;
    uint32_t No; buf >> No;

    std::map<int,linkEntry_t> final;

    for (uint32_t i = 0; i < No; ++i)
    {
        linkEntry entry = {};
        entry.priority = -1;
        entry.opened = -1;
        entry.approachingFoe = -1;
        entry.length = -1;

        // get consecutive1
        int8_t sType; buf >> sType;
        std::string consecutive1; buf >> consecutive1;
        entry.consecutive1 = consecutive1;

        // get consecutive2
        int8_t sType2; buf >> sType2;
        std::string consecutive2; buf >> consecutive2;
        entry.consecutive2 = consecutive2;

        // priority
        int8_t bType1; buf >> bType1;
        uint8_t priority; buf >> priority;
        entry.priority = priority;

        // opened
        int8_t bType2; buf >> bType2;
        uint8_t opened; buf >> opened;
        entry.opened = opened;

        // approachingFoe
        int8_t bType3; buf >> bType3;
        uint8_t approachingFoe; buf >> approachingFoe;
        entry.approachingFoe = approachingFoe;

        // get state
        int8_t sType3; buf >> sType3;
        std::string state; buf >> state;
        entry.state = state;

        // get direction
        int8_t sType4; buf >> sType4;
        std::string direction; buf >> direction;
        entry.direction = direction;

        // length
        int8_t dType1; buf >> dType1;
        double length; buf >> length;
        entry.length = length;

        // add into the map
        final.insert( std::make_pair(i,entry) );
    }

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_LANE_VARIABLE, LANE_LINKS, "laneGetLinks");

    return final;
}


std::vector<std::string> TraCI_Commands::laneGetAllowedClasses(std::string laneId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_LANE_VARIABLE, LANE_ALLOWED, "laneGetAllowedClasses");

    auto result = genericGetStringVector(CMD_GET_LANE_VARIABLE, laneId, LANE_ALLOWED, RESPONSE_GET_LANE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_LANE_VARIABLE, LANE_ALLOWED, "laneGetAllowedClasses");

    return result;
}


std::string TraCI_Commands::laneGetEdgeID(std::string laneId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_LANE_VARIABLE, LANE_EDGE_ID, "laneGetEdgeID");

    std::string result = genericGetString(CMD_GET_LANE_VARIABLE, laneId, LANE_EDGE_ID, RESPONSE_GET_LANE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_LANE_VARIABLE, LANE_EDGE_ID, "laneGetEdgeID");

    return result;
}


double TraCI_Commands::laneGetLength(std::string laneId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_LANE_VARIABLE, VAR_LENGTH, "laneGetLength");

    double result = genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_LENGTH, RESPONSE_GET_LANE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_LANE_VARIABLE, VAR_LENGTH, "laneGetLength");

    return result;
}


double TraCI_Commands::laneGetMaxSpeed(std::string laneId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_LANE_VARIABLE, VAR_MAXSPEED, "laneGetMaxSpeed");

    double result = genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_MAXSPEED, RESPONSE_GET_LANE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_LANE_VARIABLE, VAR_MAXSPEED, "laneGetMaxSpeed");

    return result;
}

double TraCI_Commands::laneGetWidth(std::string laneId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_LANE_VARIABLE, VAR_WIDTH, "laneGetWidth");

    double result = genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_WIDTH, RESPONSE_GET_LANE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_LANE_VARIABLE, VAR_WIDTH, "laneGetWidth");

    return result;
}


uint32_t TraCI_Commands::laneGetLastStepVehicleNumber(std::string laneId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_LANE_VARIABLE, LAST_STEP_VEHICLE_NUMBER, "laneGetLastStepVehicleNumber");

    uint32_t result = genericGetInt(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_VEHICLE_NUMBER, RESPONSE_GET_LANE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_LANE_VARIABLE, LAST_STEP_VEHICLE_NUMBER, "laneGetLastStepVehicleNumber");

    return result;
}


std::vector<std::string> TraCI_Commands::laneGetLastStepVehicleIDs(std::string laneId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_LANE_VARIABLE, LAST_STEP_VEHICLE_ID_LIST, "laneGetLastStepVehicleIDs");

    auto result = genericGetStringVector(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_LANE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_LANE_VARIABLE, LAST_STEP_VEHICLE_ID_LIST, "laneGetLastStepVehicleIDs");

    return result;
}


double TraCI_Commands::laneGetLastStepMeanVehicleSpeed(std::string laneId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_LANE_VARIABLE, LAST_STEP_MEAN_SPEED, "laneGetLastStepMeanVehicleSpeed");

    double result = genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_MEAN_SPEED, RESPONSE_GET_LANE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_LANE_VARIABLE, LAST_STEP_MEAN_SPEED, "laneGetLastStepMeanVehicleSpeed");

    return result;
}


double TraCI_Commands::laneGetLastStepMeanVehicleLength(std::string laneId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_LANE_VARIABLE, LAST_STEP_LENGTH, "laneGetLastStepMeanVehicleLength");

    double result = genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_LENGTH, RESPONSE_GET_LANE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_LANE_VARIABLE, LAST_STEP_LENGTH, "laneGetLastStepMeanVehicleLength");

    return result;
}


// ######################
// CMD_SET_LANE_VARIABLE
// ######################

void TraCI_Commands::laneSetMaxSpeed(std::string laneId, double value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_LANE_VARIABLE, VAR_MAXSPEED, "laneSetMaxSpeed");

    uint8_t variableId = VAR_MAXSPEED;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_LANE_VARIABLE, TraCIBuffer() << variableId << laneId << variableType << value);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_LANE_VARIABLE, VAR_MAXSPEED, "laneSetMaxSpeed");
}



// ################################################################
//                 loop detector (E1-Detectors)
// ################################################################

// ###############################
// CMD_GET_INDUCTIONLOOP_VARIABLE
// ###############################

std::vector<std::string> TraCI_Commands::LDGetIDList()
{
    record_TraCI_activity_func(commandStart, CMD_GET_INDUCTIONLOOP_VARIABLE, ID_LIST, "LDGetIDList");

    auto result = genericGetStringVector(CMD_GET_INDUCTIONLOOP_VARIABLE, "", ID_LIST, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_INDUCTIONLOOP_VARIABLE, ID_LIST, "LDGetIDList");

    return result;
}


uint32_t TraCI_Commands::LDGetIDCount(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_INDUCTIONLOOP_VARIABLE, ID_COUNT, "LDGetIDCount");

    uint32_t result = genericGetInt(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, ID_COUNT, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_INDUCTIONLOOP_VARIABLE, ID_COUNT, "LDGetIDCount");

    return result;
}


std::string TraCI_Commands::LDGetLaneID(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_INDUCTIONLOOP_VARIABLE, VAR_LANE_ID, "LDGetLaneID");

    std::string result = genericGetString(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, VAR_LANE_ID, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_INDUCTIONLOOP_VARIABLE, VAR_LANE_ID, "LDGetLaneID");

    return result;
}


double TraCI_Commands::LDGetPosition(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_INDUCTIONLOOP_VARIABLE, VAR_POSITION, "LDGetPosition");

    double result = genericGetDouble(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, VAR_POSITION, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_INDUCTIONLOOP_VARIABLE, VAR_POSITION, "LDGetPosition");

    return result;
}


uint32_t TraCI_Commands::LDGetLastStepVehicleNumber(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_VEHICLE_NUMBER, "LDGetLastStepVehicleNumber");

    uint32_t result = genericGetInt(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, LAST_STEP_VEHICLE_NUMBER, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_VEHICLE_NUMBER, "LDGetLastStepVehicleNumber");

    return result;
}


std::vector<std::string> TraCI_Commands::LDGetLastStepVehicleIDs(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_VEHICLE_ID_LIST, "LDGetLastStepVehicleIDs");

    auto result = genericGetStringVector(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_VEHICLE_ID_LIST, "LDGetLastStepVehicleIDs");

    return result;
}


double TraCI_Commands::LDGetLastStepMeanVehicleSpeed(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_MEAN_SPEED, "LDGetLastStepMeanVehicleSpeed");

    double result = genericGetDouble(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, LAST_STEP_MEAN_SPEED, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_MEAN_SPEED, "LDGetLastStepMeanVehicleSpeed");

    return result;
}


double TraCI_Commands::LDGetElapsedTimeLastDetection(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_TIME_SINCE_DETECTION, "LDGetElapsedTimeLastDetection");

    double result = genericGetDouble(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, LAST_STEP_TIME_SINCE_DETECTION, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_TIME_SINCE_DETECTION, "LDGetElapsedTimeLastDetection");

    return result;
}


std::vector<vehLD_t> TraCI_Commands::LDGetLastStepVehicleData(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_VEHICLE_DATA, "LDGetLastStepVehicleData");

    uint8_t resultTypeId = TYPE_COMPOUND;   // note: type is compound!
    uint8_t variableId = LAST_STEP_VEHICLE_DATA;
    uint8_t responseId = RESPONSE_GET_INDUCTIONLOOP_VARIABLE;

    TraCIBuffer buf = connection->query(CMD_GET_INDUCTIONLOOP_VARIABLE, TraCIBuffer() << variableId << loopId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == loopId);

    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint32_t count; buf >> count;

    // now we start getting real data that we are looking for
    std::vector<vehLD_t> res;

    // number of information packets (in 'No' variable)
    uint8_t typeI; buf >> typeI;
    uint32_t No; buf >> No;

    for (uint32_t i = 1; i <= No; ++i)
    {
        vehLD_t entry = {};

        // get vehicle id
        uint8_t typeS1; buf >> typeS1;
        std::string vId; buf >> vId;
        entry.vehID = vId;

        // get vehicle length
        uint8_t dType2; buf >> dType2;
        double len; buf >> len;
        entry.vehLength = len;

        // entryTime
        uint8_t dType3; buf >> dType3;
        double entryTime; buf >> entryTime;
        entry.entryTime = entryTime;

        // leaveTime
        uint8_t dType4; buf >> dType4;
        double leaveTime; buf >> leaveTime;
        entry.leaveTime = leaveTime;

        // vehicle type
        uint8_t dType5; buf >> dType5;
        std::string vehicleTypeID; buf >> vehicleTypeID;
        entry.vehType = vehicleTypeID;

        res.push_back(entry);
    }

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_VEHICLE_DATA, "LDGetLastStepVehicleData");

    return res;
}


double TraCI_Commands::LDGetLastStepOccupancy(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_OCCUPANCY, "LDGetLastStepOccupancy");

    double result = genericGetDouble(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x13, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_OCCUPANCY, "LDGetLastStepOccupancy");

    return result;
}


// ################################################################
//                lane area detector (E2-Detectors)
// ################################################################

// ###############################
// CMD_GET_AREAL_DETECTOR_VARIABLE
// ###############################

std::vector<std::string> TraCI_Commands::LADGetIDList()
{
    record_TraCI_activity_func(commandStart, CMD_GET_AREAL_DETECTOR_VARIABLE, ID_LIST, "LADGetIDList");

    auto result = genericGetStringVector(CMD_GET_AREAL_DETECTOR_VARIABLE, "", ID_LIST, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_AREAL_DETECTOR_VARIABLE, ID_LIST, "LADGetIDList");

    return result;
}


uint32_t TraCI_Commands::LADGetIDCount(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_AREAL_DETECTOR_VARIABLE, ID_COUNT, "LADGetIDCount");

    uint32_t result = genericGetInt(CMD_GET_AREAL_DETECTOR_VARIABLE, loopId, ID_COUNT, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_AREAL_DETECTOR_VARIABLE, ID_COUNT, "LADGetIDCount");

    return result;
}


std::string TraCI_Commands::LADGetLaneID(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_AREAL_DETECTOR_VARIABLE, VAR_LANE_ID, "LADGetLaneID");

    std::string result = genericGetString(CMD_GET_AREAL_DETECTOR_VARIABLE, loopId, VAR_LANE_ID, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_AREAL_DETECTOR_VARIABLE, VAR_LANE_ID, "LADGetLaneID");

    return result;
}


uint32_t TraCI_Commands::LADGetLastStepVehicleNumber(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_VEHICLE_NUMBER, "LADGetLastStepVehicleNumber");

    uint32_t result = genericGetInt(CMD_GET_AREAL_DETECTOR_VARIABLE, loopId, LAST_STEP_VEHICLE_NUMBER, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_VEHICLE_NUMBER, "LADGetLastStepVehicleNumber");

    return result;
}


std::vector<std::string> TraCI_Commands::LADGetLastStepVehicleIDs(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_VEHICLE_ID_LIST, "LADGetLastStepVehicleIDs");

    auto result = genericGetStringVector(CMD_GET_AREAL_DETECTOR_VARIABLE, loopId, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_VEHICLE_ID_LIST, "LADGetLastStepVehicleIDs");

    return result;
}


double TraCI_Commands::LADGetLastStepMeanVehicleSpeed(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_MEAN_SPEED, "LADGetLastStepMeanVehicleSpeed");

    double result = genericGetDouble(CMD_GET_AREAL_DETECTOR_VARIABLE, loopId, LAST_STEP_MEAN_SPEED, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_MEAN_SPEED, "LADGetLastStepMeanVehicleSpeed");

    return result;
}


uint32_t TraCI_Commands::LADGetLastStepVehicleHaltingNumber(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_VEHICLE_HALTING_NUMBER, "LADGetLastStepVehicleHaltingNumber");

    double result = genericGetInt(CMD_GET_AREAL_DETECTOR_VARIABLE, loopId, LAST_STEP_VEHICLE_HALTING_NUMBER, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_VEHICLE_HALTING_NUMBER, "LADGetLastStepVehicleHaltingNumber");

    return result;
}

double TraCI_Commands::LADGetLastStepJamLengthInMeter(std::string loopId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_AREAL_DETECTOR_VARIABLE, JAM_LENGTH_METERS, "LADGetLastStepJamLengthInMeter");

    double result = genericGetDouble(CMD_GET_AREAL_DETECTOR_VARIABLE, loopId, JAM_LENGTH_METERS, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_AREAL_DETECTOR_VARIABLE, JAM_LENGTH_METERS, "LADGetLastStepJamLengthInMeter");

    return result;
}


// ################################################################
//                          traffic light
// ################################################################

// ###################
// CMD_GET_TL_VARIABLE
// ###################

std::vector<std::string> TraCI_Commands::TLGetIDList()
{
    record_TraCI_activity_func(commandStart, CMD_GET_TL_VARIABLE, ID_LIST, "TLGetIDList");

    auto result = genericGetStringVector(CMD_GET_TL_VARIABLE, "", ID_LIST, RESPONSE_GET_TL_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_TL_VARIABLE, ID_LIST, "TLGetIDList");

    return result;
}


uint32_t TraCI_Commands::TLGetIDCount()
{
    record_TraCI_activity_func(commandStart, CMD_GET_TL_VARIABLE, ID_COUNT, "TLGetIDCount");

    uint32_t result = genericGetInt(CMD_GET_TL_VARIABLE, "", ID_COUNT, RESPONSE_GET_TL_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_TL_VARIABLE, ID_COUNT, "TLGetIDCount");

    return result;
}


std::vector<std::string> TraCI_Commands::TLGetControlledLanes(std::string TLid)
{
    record_TraCI_activity_func(commandStart, CMD_GET_TL_VARIABLE, TL_CONTROLLED_LANES, "TLGetControlledLanes");

    auto result = genericGetStringVector(CMD_GET_TL_VARIABLE, TLid, TL_CONTROLLED_LANES, RESPONSE_GET_TL_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_TL_VARIABLE, TL_CONTROLLED_LANES, "TLGetControlledLanes");

    return result;
}


std::map<int,std::vector<std::string>> TraCI_Commands::TLGetControlledLinks(std::string TLid)
{
    record_TraCI_activity_func(commandStart, CMD_GET_TL_VARIABLE, TL_CONTROLLED_LINKS, "TLGetControlledLinks");

    uint8_t resultTypeId = TYPE_COMPOUND;
    uint8_t variableId = TL_CONTROLLED_LINKS;

    TraCIBuffer buf = connection->query(CMD_GET_TL_VARIABLE, TraCIBuffer() << variableId << TLid);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == RESPONSE_GET_TL_VARIABLE);
    uint8_t varId; buf >> varId;
    ASSERT(varId == TL_CONTROLLED_LINKS);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == TLid);
    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint32_t count; buf >> count;

    uint8_t typeI; buf >> typeI;
    uint32_t No; buf >> No;

    std::map<int,std::vector<std::string>> myMap;

    for (uint32_t i = 0; i < No; ++i)
    {
        buf >> typeI;
        uint32_t No2; buf >> No2;

        buf >> typeI;
        uint32_t No3; buf >> No3;

        std::vector<std::string> lanesForThisLink;
        for(uint32_t j = 0; j < No3; ++j)
        {
            std::string id; buf >> id;
            lanesForThisLink.push_back(id);
        }

        myMap[i] = lanesForThisLink;
    }

    record_TraCI_activity_func(commandComplete, CMD_GET_TL_VARIABLE, TL_CONTROLLED_LINKS, "TLGetControlledLinks");

    return myMap;
}


std::string TraCI_Commands::TLGetProgram(std::string TLid)
{
    record_TraCI_activity_func(commandStart, CMD_GET_TL_VARIABLE, TL_CURRENT_PROGRAM, "TLGetProgram");

    std::string result = genericGetString(CMD_GET_TL_VARIABLE, TLid, TL_CURRENT_PROGRAM, RESPONSE_GET_TL_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_TL_VARIABLE, TL_CURRENT_PROGRAM, "TLGetProgram");

    return result;
}


uint32_t TraCI_Commands::TLGetPhase(std::string TLid)
{
    record_TraCI_activity_func(commandStart, CMD_GET_TL_VARIABLE, TL_CURRENT_PHASE, "TLGetPhase");

    uint32_t result = genericGetInt(CMD_GET_TL_VARIABLE, TLid, TL_CURRENT_PHASE, RESPONSE_GET_TL_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_TL_VARIABLE, TL_CURRENT_PHASE, "TLGetPhase");

    return result;
}


std::string TraCI_Commands::TLGetState(std::string TLid)
{
    record_TraCI_activity_func(commandStart, CMD_GET_TL_VARIABLE, TL_RED_YELLOW_GREEN_STATE, "TLGetState");

    std::string result = genericGetString(CMD_GET_TL_VARIABLE, TLid, TL_RED_YELLOW_GREEN_STATE, RESPONSE_GET_TL_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_TL_VARIABLE, TL_RED_YELLOW_GREEN_STATE, "TLGetState");

    return result;
}


uint32_t TraCI_Commands::TLGetPhaseDuration(std::string TLid)
{
    record_TraCI_activity_func(commandStart, CMD_GET_TL_VARIABLE, TL_PHASE_DURATION, "TLGetPhaseDuration");

    uint32_t result = genericGetInt(CMD_GET_TL_VARIABLE, TLid, TL_PHASE_DURATION, RESPONSE_GET_TL_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_TL_VARIABLE, TL_PHASE_DURATION, "TLGetPhaseDuration");

    return result;
}


uint32_t TraCI_Commands::TLGetNextSwitchTime(std::string TLid)
{
    record_TraCI_activity_func(commandStart, CMD_GET_TL_VARIABLE, TL_NEXT_SWITCH, "TLGetNextSwitchTime");

    uint32_t result = genericGetInt(CMD_GET_TL_VARIABLE, TLid, TL_NEXT_SWITCH, RESPONSE_GET_TL_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_TL_VARIABLE, TL_NEXT_SWITCH, "TLGetNextSwitchTime");

    return result;
}


// ###################
// CMD_SET_TL_VARIABLE
// ###################

void TraCI_Commands::TLSetProgram(std::string TLid, std::string value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_TL_VARIABLE, TL_PROGRAM, "TLSetProgram");

    uint8_t variableId = TL_PROGRAM;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_TL_VARIABLE, TL_PROGRAM, "TLSetProgram");
}


void TraCI_Commands::TLSetPhaseIndex(std::string TLid, int value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_TL_VARIABLE, TL_PHASE_INDEX, "TLSetPhaseIndex");

    uint8_t variableId = TL_PHASE_INDEX;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_TL_VARIABLE, TL_PHASE_INDEX, "TLSetPhaseIndex");
}


void TraCI_Commands::TLSetPhaseDuration(std::string TLid, int value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_TL_VARIABLE, TL_PHASE_DURATION, "TLSetPhaseDuration");

    uint8_t variableId = TL_PHASE_DURATION;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_TL_VARIABLE, TL_PHASE_DURATION, "TLSetPhaseDuration");
}


void TraCI_Commands::TLSetState(std::string TLid, std::string value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_TL_VARIABLE, TL_RED_YELLOW_GREEN_STATE, "TLSetState");

    uint8_t variableId = TL_RED_YELLOW_GREEN_STATE;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_TL_VARIABLE, TL_RED_YELLOW_GREEN_STATE, "TLSetState");
}


// ################################################################
//                             Junction
// ################################################################

// #########################
// CMD_GET_JUNCTION_VARIABLE
// #########################

std::vector<std::string> TraCI_Commands::junctionGetIDList()
{
    record_TraCI_activity_func(commandStart, CMD_GET_JUNCTION_VARIABLE, ID_LIST, "junctionGetIDList");

    auto result = genericGetStringVector(CMD_GET_JUNCTION_VARIABLE, "", ID_LIST, RESPONSE_GET_JUNCTION_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_JUNCTION_VARIABLE, ID_LIST, "junctionGetIDList");

    return result;
}


uint32_t TraCI_Commands::junctionGetIDCount()
{
    record_TraCI_activity_func(commandStart, CMD_GET_JUNCTION_VARIABLE, ID_COUNT, "junctionGetIDCount");

    uint32_t result = genericGetInt(CMD_GET_JUNCTION_VARIABLE, "", ID_COUNT, RESPONSE_GET_JUNCTION_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_JUNCTION_VARIABLE, ID_COUNT, "junctionGetIDCount");

    return result;
}


TraCICoord TraCI_Commands::junctionGetPosition(std::string id)
{
    record_TraCI_activity_func(commandStart, CMD_GET_JUNCTION_VARIABLE, VAR_POSITION, "junctionGetPosition");

    TraCICoord result = genericGetCoord(CMD_GET_JUNCTION_VARIABLE, id, VAR_POSITION, RESPONSE_GET_JUNCTION_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_JUNCTION_VARIABLE, VAR_POSITION, "junctionGetPosition");

    return result;
}


// ################################################################
//                               GUI
// ################################################################

// #####################
// CMD_GET_GUI_VARIABLE
// #####################

TraCICoord TraCI_Commands::GUIGetOffset(std::string viewID)
{
    record_TraCI_activity_func(commandStart, CMD_GET_GUI_VARIABLE, VAR_VIEW_OFFSET, "GUIGetOffset");

    TraCICoord result = genericGetCoord(CMD_GET_GUI_VARIABLE, viewID, VAR_VIEW_OFFSET, RESPONSE_GET_GUI_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_GUI_VARIABLE, VAR_VIEW_OFFSET, "GUIGetOffset");

    return result;
}


std::vector<double> TraCI_Commands::GUIGetBoundry(std::string viewID)
{
    record_TraCI_activity_func(commandStart, CMD_GET_GUI_VARIABLE, VAR_VIEW_BOUNDARY, "GUIGetBoundry");

    std::vector<double> result = genericGetBoundingBox(CMD_GET_GUI_VARIABLE, viewID, VAR_VIEW_BOUNDARY, RESPONSE_GET_GUI_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_GUI_VARIABLE, VAR_VIEW_BOUNDARY, "GUIGetBoundry");

    return result;
}

double TraCI_Commands::GUIGetZoom(std::string viewID)
{
    record_TraCI_activity_func(commandStart, CMD_GET_GUI_VARIABLE, VAR_VIEW_ZOOM, "GUIGetZoom");

    double result = genericGetDouble(CMD_GET_GUI_VARIABLE, viewID, VAR_VIEW_ZOOM, RESPONSE_GET_GUI_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_GUI_VARIABLE, VAR_VIEW_ZOOM, "GUIGetZoom");

    return result;
}

// #####################
// CMD_SET_GUI_VARIABLE
// #####################

void TraCI_Commands::GUISetZoom(std::string viewID, double value)
{
    record_TraCI_activity_func(commandStart, CMD_SET_GUI_VARIABLE, VAR_VIEW_ZOOM, "GUISetZoom");

    uint8_t variableId = VAR_VIEW_ZOOM;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << value);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_GUI_VARIABLE, VAR_VIEW_ZOOM, "GUISetZoom");
}


void TraCI_Commands::GUISetOffset(std::string viewID, double x, double y)
{
    record_TraCI_activity_func(commandStart, CMD_SET_GUI_VARIABLE, VAR_VIEW_OFFSET, "GUISetOffset");

    uint8_t variableId = VAR_VIEW_OFFSET;
    uint8_t variableType = POSITION_2D;

    TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << x << y);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_GUI_VARIABLE, VAR_VIEW_OFFSET, "GUISetOffset");
}


void TraCI_Commands::GUITakeScreenshot(std::string viewID, std::string filename)
{
    record_TraCI_activity_func(commandStart, CMD_SET_GUI_VARIABLE, VAR_SCREENSHOT, "GUITakeScreenshot");

    uint8_t variableId = VAR_SCREENSHOT;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << filename);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_GUI_VARIABLE, VAR_SCREENSHOT, "GUITakeScreenshot");
}


// very slow!
void TraCI_Commands::GUISetTrackVehicle(std::string viewID, std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_SET_GUI_VARIABLE, VAR_TRACK_VEHICLE, "GUISetTrackVehicle");

    uint8_t variableId = VAR_TRACK_VEHICLE;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << nodeId);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_GUI_VARIABLE, VAR_TRACK_VEHICLE, "GUISetTrackVehicle");
}


void TraCI_Commands::GUIAddView(std::string viewID)
{
    record_TraCI_activity_func(commandStart, CMD_SET_GUI_VARIABLE, 0xa7, "GUIAddView");

    // make sure the view ID is in the 'View #n' format
    if (!std::regex_match (viewID, std::regex("(View #)([[:digit:]]+)") ))
        throw omnetpp::cRuntimeError("The viewID should be in the 'View #n' format");

    uint8_t variableId = 0xa7;
    uint8_t variableType = TYPE_STRING;
    std::string init_viewID = "View #0";

    TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << init_viewID << variableType << viewID);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_GUI_VARIABLE, 0xa7, "GUIAddView");
}


// ################################################################
//                               polygon
// ################################################################

// ########################
// CMD_GET_POLYGON_VARIABLE
// ########################

std::vector<std::string> TraCI_Commands::polygonGetIDList()
{
    record_TraCI_activity_func(commandStart, CMD_GET_POLYGON_VARIABLE, ID_LIST, "polygonGetIDList");

    auto result = genericGetStringVector(CMD_GET_POLYGON_VARIABLE, "", ID_LIST, RESPONSE_GET_POLYGON_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_POLYGON_VARIABLE, ID_LIST, "polygonGetIDList");

    return result;
}


uint32_t TraCI_Commands::polygonGetIDCount()
{
    record_TraCI_activity_func(commandStart, CMD_GET_POLYGON_VARIABLE, ID_COUNT, "polygonGetIDCount");

    uint32_t result = genericGetInt(CMD_GET_POLYGON_VARIABLE, "", ID_COUNT, RESPONSE_GET_POLYGON_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_POLYGON_VARIABLE, ID_COUNT, "polygonGetIDCount");

    return result;
}


std::vector<TraCICoord> TraCI_Commands::polygonGetShape(std::string polyId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_POLYGON_VARIABLE, VAR_SHAPE, "polygonGetShape");

    auto result = genericGetCoordVector(CMD_GET_POLYGON_VARIABLE, polyId, VAR_SHAPE, RESPONSE_GET_POLYGON_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_POLYGON_VARIABLE, VAR_SHAPE, "polygonGetShape");

    return result;
}


std::string TraCI_Commands::polygonGetTypeID(std::string polyId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_POLYGON_VARIABLE, VAR_TYPE, "polygonGetTypeID");

    std::string result = genericGetString(CMD_GET_POLYGON_VARIABLE, polyId, VAR_TYPE, RESPONSE_GET_POLYGON_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_POLYGON_VARIABLE, VAR_TYPE, "polygonGetTypeID");

    return result;
}


// ########################
// CMD_SET_POLYGON_VARIABLE
// ########################

void TraCI_Commands::polygonAdd(std::string polyId, std::string polyType, const RGB color, bool filled, int32_t layer, const std::list<TraCICoord>& points)
{
    record_TraCI_activity_func(commandStart, CMD_SET_POLYGON_VARIABLE, ADD, "polygonAdd");

    TraCIBuffer p;

    p << static_cast<uint8_t>(ADD) << polyId;
    p << static_cast<uint8_t>(TYPE_COMPOUND) << static_cast<int32_t>(5);
    p << static_cast<uint8_t>(TYPE_STRING) << polyType;
    p << static_cast<uint8_t>(TYPE_COLOR) << (uint8_t)color.red << (uint8_t)color.green << (uint8_t)color.blue << (uint8_t)255 /*alpha*/;
    p << static_cast<uint8_t>(TYPE_UBYTE) << static_cast<uint8_t>(filled);
    p << static_cast<uint8_t>(TYPE_INTEGER) << layer;
    p << static_cast<uint8_t>(TYPE_POLYGON) << static_cast<uint8_t>(points.size());

    for (auto &pos : points)
        p << static_cast<double>(pos.x) << static_cast<double>(pos.y);

    TraCIBuffer buf = connection->query(CMD_SET_POLYGON_VARIABLE, p);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_POLYGON_VARIABLE, ADD, "polygonAdd");
}


void TraCI_Commands::polygonAdd(std::string polyId, std::string polyType, const RGB color, bool filled, int32_t layer, const std::list<Coord>& points)
{
    std::list<TraCICoord> points_traci;
    for(auto &n : points)
        points_traci.push_back(convertCoord_omnet2traci(n));

    polygonAdd(polyId, polyType, color, filled, layer, points_traci);
}


void TraCI_Commands::polygonSetFilled(std::string polyId, uint8_t filled)
{
    record_TraCI_activity_func(commandStart, CMD_SET_POLYGON_VARIABLE, VAR_FILL, "polygonSetFilled");

    uint8_t variableId = VAR_FILL;
    uint8_t variableType = TYPE_UBYTE;

    TraCIBuffer buf = connection->query(CMD_SET_POLYGON_VARIABLE, TraCIBuffer() << variableId << polyId << variableType << filled);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_POLYGON_VARIABLE, VAR_FILL, "polygonSetFilled");
}


// ################################################################
//                               POI
// ################################################################

void TraCI_Commands::poiAdd(std::string poiId, std::string poiType, const RGB color, int32_t layer, const TraCICoord& pos)
{
    record_TraCI_activity_func(commandStart, CMD_SET_POI_VARIABLE, ADD, "addPoi");

    TraCIBuffer p;

    p << static_cast<uint8_t>(ADD) << poiId;
    p << static_cast<uint8_t>(TYPE_COMPOUND) << static_cast<int32_t>(4);
    p << static_cast<uint8_t>(TYPE_STRING) << poiType;
    p << static_cast<uint8_t>(TYPE_COLOR) << (uint8_t)color.red << (uint8_t)color.green << (uint8_t)color.blue << (uint8_t)255 /*alpha*/;
    p << static_cast<uint8_t>(TYPE_INTEGER) << layer;
    p << pos;

    TraCIBuffer buf = connection->query(CMD_SET_POI_VARIABLE, p);
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_POI_VARIABLE, ADD, "addPoi");
}


void TraCI_Commands::poiAdd(std::string poiId, std::string poiType, const RGB color, int32_t layer, const Coord& pos)
{
    poiAdd(poiId, poiType, color, layer, convertCoord_omnet2traci(pos));
}


// ################################################################
//                               person
// ################################################################

// ##############
// CMD_GET_PERSON
// ##############

std::vector<std::string> TraCI_Commands::personGetIDList()
{
    record_TraCI_activity_func(commandStart, CMD_GET_PERSON_VARIABLE, ID_LIST, "personGetIDList");

    auto result = genericGetStringVector(CMD_GET_PERSON_VARIABLE, "", ID_LIST, RESPONSE_GET_PERSON_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_PERSON_VARIABLE, ID_LIST, "personGetIDList");

    return result;
}


uint32_t TraCI_Commands::personGetIDCount()
{
    record_TraCI_activity_func(commandStart, CMD_GET_PERSON_VARIABLE, ID_COUNT, "personGetIDCount");

    uint32_t result = genericGetInt(CMD_GET_PERSON_VARIABLE, "", ID_COUNT, RESPONSE_GET_PERSON_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_PERSON_VARIABLE, ID_COUNT, "personGetIDCount");

    return result;
}


std::string TraCI_Commands::personGetTypeID(std::string pId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_PERSON_VARIABLE, VAR_TYPE, "personGetTypeID");

    std::string result = genericGetString(CMD_GET_PERSON_VARIABLE, pId, VAR_TYPE, RESPONSE_GET_PERSON_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_PERSON_VARIABLE, VAR_TYPE, "personGetTypeID");

    return result;
}


TraCICoord TraCI_Commands::personGetPosition(std::string pId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_PERSON_VARIABLE, VAR_POSITION, "personGetPosition");

    TraCICoord result = genericGetCoord(CMD_GET_PERSON_VARIABLE, pId, VAR_POSITION, RESPONSE_GET_PERSON_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_PERSON_VARIABLE, VAR_POSITION, "personGetPosition");

    return result;
}


double TraCI_Commands::personGetAngle(std::string pId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_PERSON_VARIABLE, VAR_ANGLE, "personGetAngle");

    double result = genericGetDouble(CMD_GET_PERSON_VARIABLE, pId, VAR_ANGLE, RESPONSE_GET_PERSON_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_PERSON_VARIABLE, VAR_ANGLE, "personGetAngle");

    return result;
}


std::string TraCI_Commands::personGetEdgeID(std::string pId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_PERSON_VARIABLE, VAR_ROAD_ID, "personGetEdgeID");

    std::string result = genericGetString(CMD_GET_PERSON_VARIABLE, pId, VAR_ROAD_ID, RESPONSE_GET_PERSON_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_PERSON_VARIABLE, VAR_ROAD_ID, "personGetEdgeID");

    return result;
}


double TraCI_Commands::personGetEdgePosition(std::string pId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_PERSON_VARIABLE, VAR_LANEPOSITION, "personGetEdgePosition");

    double result = genericGetDouble(CMD_GET_PERSON_VARIABLE, pId, VAR_LANEPOSITION, RESPONSE_GET_PERSON_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_PERSON_VARIABLE, VAR_LANEPOSITION, "personGetEdgePosition");

    return result;
}


double TraCI_Commands::personGetSpeed(std::string pId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_PERSON_VARIABLE, VAR_SPEED, "personGetSpeed");

    double result = genericGetDouble(CMD_GET_PERSON_VARIABLE, pId, VAR_SPEED, RESPONSE_GET_PERSON_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_PERSON_VARIABLE, VAR_SPEED, "personGetSpeed");

    return result;
}


std::string TraCI_Commands::personGetNextEdge(std::string pId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_PERSON_VARIABLE, VAR_NEXT_EDGE, "personGetNextEdge");

    std::string result = genericGetString(CMD_GET_PERSON_VARIABLE, pId, VAR_NEXT_EDGE, RESPONSE_GET_PERSON_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_PERSON_VARIABLE, VAR_NEXT_EDGE, "personGetNextEdge");

    return result;
}

// ##############
// CMD_SET_PERSON
// ##############

void TraCI_Commands::personAdd(std::string pId, std::string edgeId, double pos, int depart, std::string pedestrianTypeId)
{
    record_TraCI_activity_func(commandStart, CMD_SET_PERSON_VARIABLE, ADD, "personAdd");

    uint8_t variableId = ADD;
    uint8_t variableType = TYPE_COMPOUND;
    uint8_t variableTypeS = TYPE_STRING;
    uint8_t variableTypeI = TYPE_INTEGER;
    uint8_t variableTypeD = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_PERSON_VARIABLE, TraCIBuffer() << variableId << pId
            << variableType << (int32_t) 4
            << variableTypeS
            << pedestrianTypeId
            << variableTypeS
            << edgeId
            << variableTypeI
            << depart
            << variableTypeD
            << pos);

    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_SET_PERSON_VARIABLE, ADD, "personAdd");
}


// ################################################################
//                              RSU
// ################################################################

std::vector<std::string> TraCI_Commands::rsuGetIDList()
{
    omnetpp::cModule *add_node_module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("addNode");
    ASSERT(add_node_module);

    std::string rsu_name = add_node_module->par("RSU_ModuleName");

    // get a pointer to the first RSU
    cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule(rsu_name.c_str(), 0);
    if(module == NULL)
        return std::vector<std::string> ();  // empty string

    std::vector<std::string> result;
    for(int i = 0; i < module->getVectorSize(); i++)
    {
        omnetpp::cModule *mod = omnetpp::getSimulation()->getSystemModule()->getSubmodule(rsu_name.c_str(), i);

        if(mod)
        {
            std::string sumoId = convertId_omnet2traci(mod->getFullName());
            ASSERT(sumoId != "");

            result.push_back(sumoId);
        }
    }

    return result;
}


uint32_t TraCI_Commands::rsuGetIDCount()
{
    omnetpp::cModule *add_node_module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("addNode");
    ASSERT(add_node_module);

    std::string rsu_name = add_node_module->par("RSU_ModuleName");

    // get a pointer to the first RSU
    cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule(rsu_name.c_str(), 0);
    if(module == NULL)
        return 0;

    // how many RSUs are in the network?
    return module->getVectorSize();
}


TraCICoord TraCI_Commands::rsuGetPosition(std::string rsuId)
{
    std::vector<std::string> rsu_list = rsuGetIDList();

    for(auto &rsu : rsu_list)
    {
        if(rsu == rsuId)
        {
            // get omnet id
            std::string omnetId = convertId_traci2omnet(rsuId);

            cModule *module = omnetpp::getSimulation()->getSystemModule()->getModuleByPath(omnetId.c_str());
            ASSERT(module);

            // get x-y coordinates of the rsu
            double xPos = module->getSubmodule("mobility")->par("x").doubleValue();
            double yPos = module->getSubmodule("mobility")->par("y").doubleValue();

            return convertCoord_omnet2traci(Coord(xPos, yPos));
        }
    }

    throw omnetpp::cRuntimeError("Cannot find RSU '%s'", rsuId.c_str());
}


// ################################################################
//                          Vehicle Platoon
// ################################################################

std::vector<std::string> TraCI_Commands::platoonGetIDList()
{
    std::vector<std::string> ids;

    // We cannot use the 'departureArrival' map
    // We need to make sure the vehicle module is currently active
    // Thus 'hosts' should be used

    // iterate over all active hosts
    for(auto &ii : this->hosts)
    {
        // hosts stores all vehicle types (bicycle, car, obstacle, etc.)
        // we have to make sure the module is a car!

        cModule *appl = ii.second->getSubmodule("appl");
        if(!appl)
            continue;

        if(!appl->hasPar("myPlnDepth"))
            continue;

        // get a pointer to the application layer of the platoon leader module
        ApplVManager *vehPtr = static_cast<ApplVManager *>(appl);
        ASSERT(vehPtr);

        if(vehPtr->getPlatoonDepth() != 0)
            continue;

        ids.push_back(ii.first);
    }

    return ids;
}


uint32_t TraCI_Commands::platoonGetIDCount()
{
    return platoonGetIDList().size();
}


omnetpp::cModule* TraCI_Commands::platoonGetLeaderModule(std::string platoonId)
{
    // get all active platoons
    auto allPlatoons = platoonGetIDList();

    auto ii = std::find(allPlatoons.begin(), allPlatoons.end(), platoonId);
    if(ii == allPlatoons.end())
        throw omnetpp::cRuntimeError("Platoon '%s' does not exist", platoonId.c_str());

    auto k = hosts.find(platoonId.c_str());
    if(k == hosts.end())
        throw omnetpp::cRuntimeError("Cannot find platoon '%s' in the hosts map", platoonId.c_str());

    // get the platoon leader module
    omnetpp::cModule *platoonLeaderModule = k->second;
    ASSERT(platoonLeaderModule);

    return platoonLeaderModule;
}


uint32_t TraCI_Commands::platoonGetSize(std::string platoonId)
{
    omnetpp::cModule *platoonLeaderModule = platoonGetLeaderModule(platoonId);

    // get the application module
    cModule *appl = platoonLeaderModule->getSubmodule("appl");
    ASSERT(appl);

    return appl->par("plnSize").intValue();
}


uint32_t TraCI_Commands::platoonGetOptSize(std::string platoonId)
{
    omnetpp::cModule *platoonLeaderModule = platoonGetLeaderModule(platoonId);

    // get the application module
    cModule *appl = platoonLeaderModule->getSubmodule("appl");
    ASSERT(appl);

    return appl->par("optPlatoonSize").intValue();
}


uint32_t TraCI_Commands::platoonGetMaxSize(std::string platoonId)
{
    omnetpp::cModule *platoonLeaderModule = platoonGetLeaderModule(platoonId);

    // get the application module
    cModule *appl = platoonLeaderModule->getSubmodule("appl");
    ASSERT(appl);

    return appl->par("maxPlatoonSize").intValue();
}


std::vector<std::string> TraCI_Commands::platoonGetMembers(std::string platoonId)
{
    omnetpp::cModule *platoonLeaderModule = platoonGetLeaderModule(platoonId);

    // get the application module
    cModule *appl = platoonLeaderModule->getSubmodule("appl");
    ASSERT(appl);

    // get a pointer to the application layer of the platoon leader module
    ApplVManager *vehPtr = static_cast<ApplVManager *>(appl);
    ASSERT(vehPtr);

    std::vector<std::string> members;
    for(auto &member : vehPtr->getPlatoonMembers())
        members.push_back(member);

    return members;
}


bool TraCI_Commands::platoonIsPltMgmtProtActive(std::string platoonId)
{
    omnetpp::cModule *platoonLeaderModule = platoonGetLeaderModule(platoonId);

    // get the application module
    cModule *appl = platoonLeaderModule->getSubmodule("appl");
    ASSERT(appl);

    // get the platoon leader mode
    int plnMode = appl->par("plnMode").intValue();

    return (plnMode == 3);
}


// ################################################################
//                              Obstacle
// ################################################################

std::vector<std::string> TraCI_Commands::obstacleGetIDList()
{
    omnetpp::cModule *add_node_module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("addNode");
    ASSERT(add_node_module);

    std::string obstacle_name = add_node_module->par("obstacle_ModuleName");

    // get a pointer to the first obstacle
    cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule(obstacle_name.c_str(), 0);
    if(module == NULL)
        return std::vector<std::string> ();  // empty string

    std::vector<std::string> result;
    for(int i = 0; i < module->getVectorSize(); i++)
    {
        std::string omnetID = omnetpp::getSimulation()->getSystemModule()->getSubmodule(obstacle_name.c_str(), i)->getFullName();
        std::string sumoId = convertId_omnet2traci(omnetID);
        ASSERT(sumoId != "");
        result.push_back(sumoId);
    }

    return result;
}


uint32_t TraCI_Commands::obstacleGetIDCount()
{
    return obstacleGetIDList().size();
}


TraCICoord TraCI_Commands::obstacleGetPosition(std::string obstacleId)
{
    std::vector<std::string> obstacle_list = obstacleGetIDList();

    for(auto &obstacle : obstacle_list)
    {
        if(obstacle == obstacleId)
        {
            // get omnet id
            std::string omnetId = convertId_traci2omnet(obstacle);

            cModule *module = omnetpp::getSimulation()->getSystemModule()->getModuleByPath(omnetId.c_str());
            ASSERT(module);

            // get x-y coordinates of the obstacle
            double xPos = std::atof( module->getDisplayString().getTagArg("p",0) );
            double yPos = std::atof( module->getDisplayString().getTagArg("p",1) );

            return convertCoord_omnet2traci(Coord(xPos, yPos));
        }
    }

    throw omnetpp::cRuntimeError("Cannot find Obstacle '%s'", obstacleId.c_str());
}


std::string TraCI_Commands::obstacleGetEdgeID(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_ROAD_ID, "obstacleGetEdgeID");

    std::string result = genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROAD_ID, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_ROAD_ID, "obstacleGetEdgeID");

    return result;
}


std::string TraCI_Commands::obstacleGetLaneID(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_LANE_ID, "obstacleGetLaneID");

    std::string result = genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_ID, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_LANE_ID, "obstacleGetLaneID");

    return result;
}


uint32_t TraCI_Commands::obstacleGetLaneIndex(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_LANE_INDEX, "obstacleGetLaneIndex");

    int32_t result = genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_INDEX, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_LANE_INDEX, "obstacleGetLaneIndex");

    return result;
}


double TraCI_Commands::obstacleGetLanePosition(std::string nodeId)
{
    record_TraCI_activity_func(commandStart, CMD_GET_VEHICLE_VARIABLE, VAR_LANEPOSITION, "obstacleGetLanePosition");

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANEPOSITION, RESPONSE_GET_VEHICLE_VARIABLE);

    record_TraCI_activity_func(commandComplete, CMD_GET_VEHICLE_VARIABLE, VAR_LANEPOSITION, "obstacleGetLanePosition");

    return result;
}


// ################################################################
//                      SUMO-OMNET conversion
// ################################################################

std::string TraCI_Commands::convertId_traci2omnet(std::string SUMOid) const
{
    auto ii = SUMOid_OMNETid_mapping.find(SUMOid);
    if(ii != SUMOid_OMNETid_mapping.end())
        return ii->second;
    else
        return "";
}


std::string TraCI_Commands::convertId_omnet2traci(std::string omnetid) const
{
    auto ii = OMNETid_SUMOid_mapping.find(omnetid);
    if(ii != OMNETid_SUMOid_mapping.end())
        return ii->second;
    else
        return "";
}


Coord TraCI_Commands::convertCoord_traci2omnet(TraCICoord coord) const
{
    return Coord(coord.x - netbounds1.x + margin, (netbounds2.y - netbounds1.y) - (coord.y - netbounds1.y) + margin);
}


TraCICoord TraCI_Commands::convertCoord_omnet2traci(Coord coord) const
{
    return TraCICoord(coord.x + netbounds1.x - margin, (netbounds2.y - netbounds1.y) - (coord.y - netbounds1.y) + margin);
}


double TraCI_Commands::convertAngle_traci2omnet(double angle) const
{
    // rotate angle so 0 is east (in TraCI's angle interpretation 0 is north, 90 is east)
    angle = 90 - angle;

    // convert to rad
    angle = angle * M_PI / 180.0;

    // normalize angle to -M_PI <= angle < M_PI
    while (angle < -M_PI) angle += 2 * M_PI;
    while (angle >= M_PI) angle -= 2 * M_PI;

    return angle;
}


double TraCI_Commands::convertAngle_omnet2traci(double angle) const
{
    // convert to degrees
    angle = angle * 180 / M_PI;

    // rotate angle so 0 is south (in OMNeT++'s angle interpretation 0 is east, 90 is north)
    angle = 90 - angle;

    // normalize angle to -180 <= angle < 180
    while (angle < -180) angle += 360;
    while (angle >= 180) angle -= 360;

    return angle;
}


// ################################################################
//                       SUMO directory
// ################################################################

boost::filesystem::path TraCI_Commands::getFullPath_SUMOApplication()
{
    std::string sumoAppl = par("SUMOapplication").stringValue();

    std::ostringstream str;
    str << boost::format("exec bash -c 'readlink -f $(which %1%)'") % sumoAppl;

    FILE* pip = popen(str.str().c_str(), "r");
    if (!pip)
        throw omnetpp::cRuntimeError("cannot open pipe: ", str.str().c_str());

    char output[10000];
    memset (output, 0, sizeof(output));
    if (!fgets(output, sizeof(output), pip))
    {
        LOG_WARNING << boost::format(">>> '%s' application is not in your PATH. \n") % sumoAppl;
        LOG_WARNING << boost::format("    1. Have you launched OMNET++ IDE from the Desktop launcher? If yes, then you probably forgot to follow 'Step 4' of installation. \n");
        LOG_WARNING << boost::format("    2. Make sure the 'Network.TraCI.SUMOapplication' parameter in set correctly. \n");
        LOG_WARNING << std::flush;

        fclose(pip);
        throw omnetpp::cRuntimeError("Cannot run SUMO application");
    }

    fclose(pip);

    std::string SUMOexeFullPath = std::string(output);
    // remove new line character at the end
    SUMOexeFullPath.erase(std::remove(SUMOexeFullPath.begin(), SUMOexeFullPath.end(), '\n'), SUMOexeFullPath.end());

    // check if this file exists?
    if( !boost::filesystem::exists(SUMOexeFullPath) )
        throw omnetpp::cRuntimeError("SUMO executable not found at '%s'", SUMOexeFullPath.c_str());

    return SUMOexeFullPath;
}


boost::filesystem::path TraCI_Commands::getFullPath_SUMOConfig()
{
    boost::filesystem::path config_FullPath = omnetpp::getEnvir()->getConfig()->getConfigEntry("description").getBaseDirectory();
    std::string SUMOconfig = par("SUMOconfig").stringValue();
    boost::filesystem::path SUMOconfigFullPath = config_FullPath / SUMOconfig;

    if( !boost::filesystem::exists(SUMOconfigFullPath) || !boost::filesystem::is_regular_file(SUMOconfigFullPath) )
        throw omnetpp::cRuntimeError("SUMO configure file is not found in '%s'", SUMOconfigFullPath.string().c_str());

    return SUMOconfigFullPath;
}


bool TraCI_Commands::IsGUI()
{
    std::string sumo_application = this->par("SUMOapplication").stdstringValue();

    if(sumo_application == "sumo" || sumo_application == "sumoD")
        return false;
    else if(sumo_application == "sumo-gui" || sumo_application == "sumo-guiD")
        return true;
    else
        throw omnetpp::cRuntimeError("SUMO application '%s' is not recognized. Make sure the Network.TraCI.SUMOapplication parameter in set correctly.", sumo_application.c_str());
}


// ################################################################
//                    Hardware in the loop (HIL)
// ################################################################

std::string TraCI_Commands::ip2vehicleId(std::string ipAddress) const
{
    auto ii = ipv4_SUMOid_mapping.find(ipAddress);
    if(ii != ipv4_SUMOid_mapping.end())
        return ii->second;
    else
        return "";
}


std::string TraCI_Commands::vehicleId2ip(std::string id) const
{
    auto ii = SUMOid_ipv4_mapping.find(id);
    if(ii != SUMOid_ipv4_mapping.end())
        return ii->second;
    else
        return "";
}


// ################################################################
//                            Mapping
// ################################################################

void TraCI_Commands::addMapping(std::string SUMOID, std::string OMNETID)
{
    // save mapping of SUMO id and OMNET++ id
    auto i1 = SUMOid_OMNETid_mapping.find(SUMOID);
    if(i1 != SUMOid_OMNETid_mapping.end())
        throw omnetpp::cRuntimeError("SUMO id %s already exists in the network!", SUMOID.c_str());
    SUMOid_OMNETid_mapping[SUMOID] = OMNETID;

    // save mapping of OMNET++ id and SUMO id
    auto i2 = OMNETid_SUMOid_mapping.find(OMNETID);
    if(i2 != OMNETid_SUMOid_mapping.end())
        throw omnetpp::cRuntimeError("OMNET++ id %s already exists in the network!", OMNETID.c_str());
    OMNETid_SUMOid_mapping[OMNETID] = SUMOID;
}


void TraCI_Commands::removeMapping(std::string SUMOID)
{
    // remove mapping of SUMO id and OMNET++ id
    auto i1 = SUMOid_OMNETid_mapping.find(SUMOID);
    if(i1 == SUMOid_OMNETid_mapping.end())
        throw omnetpp::cRuntimeError("SUMO id %s does not exist in the network!", SUMOID.c_str());
    std::string OMNETID = i1->second; // save omnet id before deleting
    SUMOid_OMNETid_mapping.erase(i1);

    // remove mapping of OMNET++ id and SUMO id
    auto i2 = OMNETid_SUMOid_mapping.find(OMNETID);
    if(i2 == OMNETid_SUMOid_mapping.end())
        throw omnetpp::cRuntimeError("OMNET++ id %s does not exist in the network!", OMNETID.c_str());
    OMNETid_SUMOid_mapping.erase(i2);
}


void TraCI_Commands::addMappingEmulated(std::string ipAddress, std::string vID)
{
    if(vID == "")
        throw omnetpp::cRuntimeError("vehicle id is empty in addMappingEmulated");

    if(ipAddress == "")
        throw omnetpp::cRuntimeError("ip address is empty in addMappingEmulated");

    auto it = SUMOid_ipv4_mapping.find(vID);
    if(it != SUMOid_ipv4_mapping.end())
        throw omnetpp::cRuntimeError("vehicle '%s' is already an emulated vehicle", vID.c_str());
    SUMOid_ipv4_mapping[vID] = ipAddress;

    auto itt = ipv4_SUMOid_mapping.find(ipAddress);
    if(itt != ipv4_SUMOid_mapping.end())
        throw omnetpp::cRuntimeError("ip '%s' is already assigned to vehicle '%s'", ipAddress.c_str(), itt->second.c_str());
    ipv4_SUMOid_mapping[ipAddress] = vID;
}


void TraCI_Commands::removeMappingEmulated(std::string vID)
{
    if(vID == "")
        throw omnetpp::cRuntimeError("vehicle id is empty in removeMappingEmulated");

    auto ii = SUMOid_ipv4_mapping.find(vID);
    if(ii != SUMOid_ipv4_mapping.end())
    {
        std::string ipv4 = ii->second;
        SUMOid_ipv4_mapping.erase(ii);

        // then remove mapping of IPv4 and OMNET id
        auto jj = ipv4_SUMOid_mapping.find(ipv4);
        if(jj == ipv4_SUMOid_mapping.end())
            throw omnetpp::cRuntimeError("IP address '%s' does not exist in the map!", ipv4.c_str());
        ipv4_SUMOid_mapping.erase(jj);
    }
}


// ################################################################
//                     protected methods!
// ################################################################

std::pair<uint32_t, std::string> TraCI_Commands::getVersion()
{
    record_TraCI_activity_func(commandStart, CMD_GETVERSION, 0xff, "getVersion");

    TraCIBuffer buf = connection->query(CMD_GETVERSION, TraCIBuffer());

    uint8_t cmdLength; buf >> cmdLength;
    uint8_t commandResp; buf >> commandResp;
    ASSERT(commandResp == CMD_GETVERSION);
    uint32_t apiVersion; buf >> apiVersion;
    std::string serverVersion; buf >> serverVersion;
    ASSERT(buf.eof());

    record_TraCI_activity_func(commandComplete, CMD_GETVERSION, 0xff, "getVersion");

    return std::make_pair(apiVersion, serverVersion);
}


void TraCI_Commands::close_TraCI_connection()
{
    record_TraCI_activity_func(commandStart, CMD_CLOSE, 0xff, "simulationTerminate");

    TraCIBuffer buf = connection->query(CMD_CLOSE, TraCIBuffer());

    record_TraCI_activity_func(commandComplete, CMD_CLOSE, 0xff, "simulationTerminate");
}


// proceed SUMO simulation to targetTime
std::pair<TraCIBuffer, uint32_t> TraCI_Commands::simulationTimeStep(uint32_t targetTime)
{
    record_TraCI_activity_func(commandStart, CMD_SIMSTEP2, 0xff, "simulationTimeStep");

    TraCIBuffer buf = connection->query(CMD_SIMSTEP2, TraCIBuffer() << targetTime);

    uint32_t count;
    buf >> count;  // count: number of subscription results

    record_TraCI_activity_func(commandComplete, CMD_SIMSTEP2, 0xff, "simulationTimeStep");

    return std::make_pair(buf, count);
}


void TraCI_Commands::recordDeparture(std::string SUMOID)
{
    auto it = departureArrival.find(SUMOID);
    if(it == departureArrival.end())
    {
        departureArrivalEntry_t entry = {omnetpp::simTime().dbl(), -1};
        departureArrival[SUMOID] = entry;
    }
    else // newly departed vehicle might have the same name as one of the previous arrived vehicles (equilibrium is on?)
    {
        it->second.departure = omnetpp::simTime().dbl();
        it->second.arrival = -1;
    }
}


void TraCI_Commands::recordArrival(std::string SUMOID)
{
    auto it = departureArrival.find(SUMOID);
    if(it == departureArrival.end())
        throw omnetpp::cRuntimeError("cannot find %s in the departureArrival map!", SUMOID.c_str());

    it->second.arrival = omnetpp::simTime().dbl();
}


// ################################################################
//                    generic methods for getters
// ################################################################

double TraCI_Commands::genericGetDouble(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_DOUBLE;
    double res;

    TraCIBuffer buf = connection->query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    buf >> res;

    ASSERT(buf.eof());

    return res;
}


int32_t TraCI_Commands::genericGetInt(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_INTEGER;
    int32_t res;

    TraCIBuffer buf = connection->query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    buf >> res;

    ASSERT(buf.eof());

    return res;
}


std::string TraCI_Commands::genericGetString(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_STRING;
    std::string res;

    TraCIBuffer buf = connection->query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    buf >> res;

    ASSERT(buf.eof());

    return res;
}


std::vector<std::string> TraCI_Commands::genericGetStringVector(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_STRINGLIST;
    std::vector<std::string> res;

    TraCIBuffer buf = connection->query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint32_t count; buf >> count;
    for (uint32_t i = 0; i < count; i++) {
        std::string id; buf >> id;
        res.push_back(id);
    }

    // todo: in 'laneGetAllowedClasses' buf is not empty
    // ASSERT(buf.eof());

    return res;
}


TraCICoord TraCI_Commands::genericGetCoord(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = POSITION_2D;
    double x;
    double y;

    TraCIBuffer buf = connection->query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    buf >> x;
    buf >> y;

    ASSERT(buf.eof());

    return TraCICoord(x, y);
}


std::vector<TraCICoord> TraCI_Commands::genericGetCoordVector(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_POLYGON;
    std::vector<TraCICoord> res;

    TraCIBuffer buf = connection->query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint8_t count; buf >> count;
    for (uint32_t i = 0; i < count; i++) {
        double x; buf >> x;
        double y; buf >> y;
        res.push_back(TraCICoord(x, y));
    }

    ASSERT(buf.eof());

    return res;
}


uint8_t TraCI_Commands::genericGetUnsignedByte(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_UBYTE;
    int8_t res;

    TraCIBuffer buf = connection->query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    buf >> res;

    ASSERT(buf.eof());

    return res;
}


// Boundary Box (4 doubles)
std::vector<double> TraCI_Commands::genericGetBoundingBox(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_BOUNDINGBOX;
    double LowerLeftX;
    double LowerLeftY;
    double UpperRightX;
    double UpperRightY;
    std::vector<double> res;

    TraCIBuffer buf = connection->query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    std::string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == objectId);
    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);

    buf >> LowerLeftX;
    res.push_back(LowerLeftX);

    buf >> LowerLeftY;
    res.push_back(LowerLeftY);

    buf >> UpperRightX;
    res.push_back(UpperRightX);

    buf >> UpperRightY;
    res.push_back(UpperRightY);

    ASSERT(buf.eof());

    return res;
}


// ################################################################
//               logging TraCI commands exchange
// ################################################################

void TraCI_Commands::record_TraCI_activity_func(TraCI_Commands::action_t state, uint8_t commandGroupId, uint8_t commandId, std::string commandName)
{
    if(!record_TraCI_activity)
        return;

    if(state == commandStart)
    {
        Htime_t startTime = std::chrono::high_resolution_clock::now();

        TraCIcommandEntry_t entry;

        entry.timeStamp = omnetpp::simTime().dbl();
        entry.sentAt = startTime;
        entry.completeAt = startTime;  // same as startTime
        entry.senderModule = "";
        entry.commandGroupId = commandGroupId;
        entry.commandId = commandId;
        entry.commandName = commandName;

        exchangedTraCIcommands.push_back(entry);
    }
    else if(state == commandComplete)
    {
        // save time here (do not include vector search in the command finish time)
        Htime_t endTime = std::chrono::high_resolution_clock::now();

        bool found = false;
        for (auto it = exchangedTraCIcommands.rbegin(); it != exchangedTraCIcommands.rend(); ++it)
        {
            if(it->commandGroupId == commandGroupId && it->commandId == commandId)
            {
                found = true;
                it->completeAt = endTime;
                break;
            }
        }

        if(!found)
            throw omnetpp::cRuntimeError("pair (%x, %x) is not found in exchangedTraCIcommands \n", commandGroupId, commandId);
    }
    else
        throw omnetpp::cRuntimeError("unknown state '%d' in record_TraCI_activity_func method", state);
}


void TraCI_Commands::save_TraCI_activity_toFile()
{
    if(exchangedTraCIcommands.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_TraCIActivity.txt") % currentRun;

    boost::filesystem::path filePath ("results");
    filePath /= fileName.str();

    FILE *filePtr = fopen (filePath.c_str(), "w");
    if (!filePtr)
        throw omnetpp::cRuntimeError("Cannot create file '%s'", filePath.c_str());

    // write simulation parameters at the beginning of the file
    {
        // get the current config name
        std::string configName = omnetpp::getEnvir()->getConfigEx()->getVariable("configname");

        std::string iniFile = omnetpp::getEnvir()->getConfigEx()->getVariable("inifile");

        // PID of the simulation process
        std::string processid = omnetpp::getEnvir()->getConfigEx()->getVariable("processid");

        // globally unique identifier for the run, produced by
        // concatenating the configuration name, run number, date/time, etc.
        std::string runID = omnetpp::getEnvir()->getConfigEx()->getVariable("runid");

        // get number of total runs in this config
        int totalRun = omnetpp::getEnvir()->getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

        // get configuration nametion variables
        std::vector<std::string> iterVar = omnetpp::getEnvir()->getConfigEx()->getConfigChain(configName.c_str());

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "iniFile         %s\n", iniFile.c_str());
        fprintf (filePtr, "processID       %s\n", processid.c_str());
        fprintf (filePtr, "runID           %s\n", runID.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n", iterVar[0].c_str());
        fprintf (filePtr, "sim timeStep    %u ms\n", simulationGetDelta());
        fprintf (filePtr, "startDateTime   %s\n", simulationGetStartTime_str().c_str());
        fprintf (filePtr, "endDateTime     %s\n", simulationGetEndTime_str().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", simulationGetDuration_str().c_str());
    }

    // write header
    fprintf (filePtr, "%-15s", "sentAt");
    fprintf (filePtr, "%-40s", "cmdName");
    fprintf (filePtr, "%-20s", "cmdGroupId");
    fprintf (filePtr, "%-19s", "cmdId");
    fprintf (filePtr, "%-15s \n\n", "duration(ms)");

    // write body
    double oldTime = -1;
    for(auto &y : exchangedTraCIcommands)
    {
        // make the log more readable :)
        if(y.timeStamp != oldTime)
        {
            fprintf(filePtr, "\n");
            oldTime = y.timeStamp;
        }

        std::chrono::duration<double, std::milli> fp_ms = y.completeAt - y.sentAt;
        double duration = fp_ms.count();

        fprintf (filePtr, "%-15.3f", y.timeStamp);
        fprintf (filePtr, "%-40s", y.commandName.c_str());
        fprintf (filePtr, "0x%-20x", y.commandGroupId);
        fprintf (filePtr, "0x%-15x", y.commandId);
        fprintf (filePtr, "%-15.3f \n", duration);
    }

    fclose(filePtr);
}


void TraCI_Commands::save_TraCI_activity_summary_toFile()
{
    if(exchangedTraCIcommands.empty())
        return;

    int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

    std::ostringstream fileName;
    fileName << boost::format("%03d_TraCIActivitySummary.txt") % currentRun;

    boost::filesystem::path filePath ("results");
    filePath /= fileName.str();

    FILE *filePtr = fopen (filePath.c_str(), "w");
    if (!filePtr)
        throw omnetpp::cRuntimeError("Cannot create file '%s'", filePath.c_str());

    // write simulation parameters at the beginning of the file
    {
        // get the current config name
        std::string configName = omnetpp::getEnvir()->getConfigEx()->getVariable("configname");

        std::string iniFile = omnetpp::getEnvir()->getConfigEx()->getVariable("inifile");

        // PID of the simulation process
        std::string processid = omnetpp::getEnvir()->getConfigEx()->getVariable("processid");

        // globally unique identifier for the run, produced by
        // concatenating the configuration name, run number, date/time, etc.
        std::string runID = omnetpp::getEnvir()->getConfigEx()->getVariable("runid");

        // get number of total runs in this config
        int totalRun = omnetpp::getEnvir()->getConfigEx()->getNumRunsInConfig(configName.c_str());

        // get the current run number
        int currentRun = omnetpp::getEnvir()->getConfigEx()->getActiveRunNumber();

        // get configuration nametion variables
        std::vector<std::string> iterVar = omnetpp::getEnvir()->getConfigEx()->getConfigChain(configName.c_str());

        // write to file
        fprintf (filePtr, "configName      %s\n", configName.c_str());
        fprintf (filePtr, "iniFile         %s\n", iniFile.c_str());
        fprintf (filePtr, "processID       %s\n", processid.c_str());
        fprintf (filePtr, "runID           %s\n", runID.c_str());
        fprintf (filePtr, "totalRun        %d\n", totalRun);
        fprintf (filePtr, "currentRun      %d\n", currentRun);
        fprintf (filePtr, "currentConfig   %s\n", iterVar[0].c_str());
        fprintf (filePtr, "sim timeStep    %u ms\n", simulationGetDelta());
        fprintf (filePtr, "startDateTime   %s\n", simulationGetStartTime_str().c_str());
        fprintf (filePtr, "endDateTime     %s\n", simulationGetEndTime_str().c_str());
        fprintf (filePtr, "duration        %s\n\n\n", simulationGetDuration_str().c_str());
    }

    struct TraCIcommandEntrySummary_t
    {
        std::string cmdName;
        uint32_t cmdCount;
        double totalDuration;
    };

    std::map<std::string /*command name*/, TraCIcommandEntrySummary_t> cmd_summary;

    for(auto &y : exchangedTraCIcommands)
    {
        std::chrono::duration<double, std::milli> fp_ms = y.completeAt - y.sentAt;
        double duration = fp_ms.count();

        auto ii = cmd_summary.find(y.commandName);
        if(ii == cmd_summary.end())
        {
            TraCIcommandEntrySummary_t entry;
            entry.cmdName = y.commandName;
            entry.cmdCount = 1;
            entry.totalDuration = duration;

            cmd_summary[y.commandName] = entry;
        }
        else
        {
            ii->second.cmdCount++;
            ii->second.totalDuration += duration;
        }
    }

    // copy cmd_summary into vector
    std::vector<TraCIcommandEntrySummary_t> cmd_summary_vec;
    for(auto &y : cmd_summary)
    {
        TraCIcommandEntrySummary_t entry;
        entry.cmdName = y.first;
        entry.cmdCount = y.second.cmdCount;
        entry.totalDuration = y.second.totalDuration;

        cmd_summary_vec.push_back(entry);
    }

    // sort cmd_summary_vec by totalDuration
    std::sort(cmd_summary_vec.begin(), cmd_summary_vec.end(),
            [](const TraCIcommandEntrySummary_t &a, const TraCIcommandEntrySummary_t &b) {
        return a.totalDuration > b.totalDuration;
    });

    // write header
    fprintf (filePtr, "%-40s", "cmdName");
    fprintf (filePtr, "%-15s", "numCalls");
    fprintf (filePtr, "%-15s \n\n", "totalDuration(ms)");

    // write body
    for(auto &y : cmd_summary_vec)
    {
        fprintf (filePtr, "%-40s", y.cmdName.c_str());
        fprintf (filePtr, "%-15u", y.cmdCount);
        fprintf (filePtr, "%-15.3f \n", y.totalDuration);
    }

    fclose(filePtr);
}

}

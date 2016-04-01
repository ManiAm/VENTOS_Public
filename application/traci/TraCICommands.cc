/****************************************************************************/
/// @file    TraCI_Commands.cc
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

#include "TraCICommands.h"
#include "TraCIConstants.h"

#include <cmath>
#include <iomanip>
#include "algorithm"

// un-defining ev!
// why? http://stackoverflow.com/questions/24103469/cant-include-the-boost-filesystem-header
#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

namespace VENTOS {

Define_Module(VENTOS::TraCI_Commands);

TraCI_Commands::~TraCI_Commands()
{

}


void TraCI_Commands::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == 0)
    {
        logTraCIcommands = par("logTraCIcommands").boolValue();
        margin = par("margin").longValue();
    }
}


void TraCI_Commands::finish()
{
    if(logTraCIcommands)
        TraCIexchangeToFile();
}


void TraCI_Commands::handleMessage(cMessage *msg)
{

}


// ################################################################
//                           Python interaction
// ################################################################

std::pair<uint32_t, std::string> TraCI_Commands::getVersion()
{
    updateTraCIlog("commandStart", CMD_GETVERSION, 0xff);

    bool success = false;
    TraCIBuffer buf = connection->queryOptional(CMD_GETVERSION, TraCIBuffer(), success);

    if (!success)
    {
        ASSERT(buf.eof());
        return std::make_pair(0, "(unknown)");
    }

    uint8_t cmdLength; buf >> cmdLength;
    uint8_t commandResp; buf >> commandResp;
    ASSERT(commandResp == CMD_GETVERSION);
    uint32_t apiVersion; buf >> apiVersion;
    std::string serverVersion; buf >> serverVersion;
    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_GETVERSION, 0xff);

    return std::make_pair(apiVersion, serverVersion);
}


// ################################################################
//                      simulation control
// ################################################################

void TraCI_Commands::simulationTerminate()
{
    updateTraCIlog("commandStart", CMD_CLOSE, 0xff);

    TraCIBuffer buf = connection->query(CMD_CLOSE, TraCIBuffer() << 0);

    uint32_t count;
    buf >> count;

    updateTraCIlog("commandComplete", CMD_CLOSE, 0xff);
}


// proceed SUMO simulation to targetTime
std::pair<TraCIBuffer, uint32_t> TraCI_Commands::simulationTimeStep(uint32_t targetTime)
{
    updateTraCIlog("commandStart", CMD_SIMSTEP2, 0xff);

    TraCIBuffer buf = connection->query(CMD_SIMSTEP2, TraCIBuffer() << targetTime);
    uint32_t count;
    buf >> count;  // count: number of subscription results

    updateTraCIlog("commandComplete", CMD_SIMSTEP2, 0xff);

    return std::make_pair(buf, count);
}


// ################################################################
//                            subscription
// ################################################################

// CMD_SUBSCRIBE_SIM_VARIABLE
TraCIBuffer TraCI_Commands::simulationSubscribe(uint32_t beginTime, uint32_t endTime, std::string objectId, std::vector<uint8_t> variables)
{
    updateTraCIlog("commandStart", CMD_SUBSCRIBE_SIM_VARIABLE, 0xff);

    TraCIBuffer p;
    p << beginTime << endTime << objectId << (uint8_t)variables.size();
    for(uint8_t i : variables)
        p << i;

    TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_SIM_VARIABLE, p);

    updateTraCIlog("commandComplete", CMD_SUBSCRIBE_SIM_VARIABLE, 0xff);

    return buf;
}


// CMD_SUBSCRIBE_VEHICLE_VARIABLE
TraCIBuffer TraCI_Commands::vehicleSubscribe(uint32_t beginTime, uint32_t endTime, std::string objectId, std::vector<uint8_t> variables)
{
    updateTraCIlog("commandStart", CMD_SUBSCRIBE_VEHICLE_VARIABLE, 0xff);

    TraCIBuffer p;
    p << beginTime << endTime << objectId << (uint8_t)variables.size();
    for(uint8_t i : variables)
        p << i;

    TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_VEHICLE_VARIABLE, p);

    updateTraCIlog("commandComplete", CMD_SUBSCRIBE_VEHICLE_VARIABLE, 0xff);

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
    updateTraCIlog("commandStart", CMD_GET_SIM_VARIABLE, VAR_LOADED_VEHICLES_NUMBER);

    // query road network boundaries
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

    updateTraCIlog("commandComplete", CMD_GET_SIM_VARIABLE, VAR_LOADED_VEHICLES_NUMBER);

    return val;
}


std::list<std::string> TraCI_Commands::simulationGetLoadedVehiclesIDList()
{
    updateTraCIlog("commandStart", CMD_GET_SIM_VARIABLE, VAR_LOADED_VEHICLES_IDS);

    uint8_t resultTypeId = TYPE_STRINGLIST;
    std::list<std::string> res;

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

    updateTraCIlog("commandComplete", CMD_GET_SIM_VARIABLE, VAR_LOADED_VEHICLES_IDS);

    return res;
}


double* TraCI_Commands::simulationGetNetBoundary()
{
    updateTraCIlog("commandStart", CMD_GET_SIM_VARIABLE, VAR_NET_BOUNDING_BOX);

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

    double *boundaries = new double[4] ;

    buf >> boundaries[0];  // x1
    buf >> boundaries[1];  // y1
    buf >> boundaries[2];  // x2
    buf >> boundaries[3];  // y2

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_GET_SIM_VARIABLE, VAR_NET_BOUNDING_BOX);

    return boundaries;
}


uint32_t TraCI_Commands::simulationGetMinExpectedNumber()
{
    updateTraCIlog("commandStart", CMD_GET_SIM_VARIABLE, VAR_MIN_EXPECTED_VEHICLES);

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

    updateTraCIlog("commandComplete", CMD_GET_SIM_VARIABLE, VAR_MIN_EXPECTED_VEHICLES);

    return val;
}


uint32_t TraCI_Commands::simulationGetArrivedNumber()
{
    updateTraCIlog("commandStart", CMD_GET_SIM_VARIABLE, VAR_ARRIVED_VEHICLES_NUMBER);

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

    updateTraCIlog("commandComplete", CMD_GET_SIM_VARIABLE, VAR_ARRIVED_VEHICLES_NUMBER);

    return val;
}


// ################################################################
//                            vehicle
// ################################################################

// #########################
// CMD_GET_VEHICLE_VARIABLE
// #########################

// gets a list of all vehicles in the network (alphabetically!!!)
std::list<std::string> TraCI_Commands::vehicleGetIDList()
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_VEHICLE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, ID_LIST);
    return result;
}


uint32_t TraCI_Commands::vehicleGetIDCount()
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, ID_COUNT);

    int32_t result = genericGetInt(CMD_GET_VEHICLE_VARIABLE, "", ID_COUNT, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, ID_COUNT);
    return result;
}


double TraCI_Commands::vehicleGetSpeed(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE,VAR_SPEED);

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_SPEED, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE,VAR_SPEED);
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
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_STOPSTATE);

    uint8_t result = genericGetUnsignedByte(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_STOPSTATE, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_STOPSTATE);
    return result;
}


Coord TraCI_Commands::vehicleGetPosition(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_POSITION);

    Coord result = genericGetCoordv2(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_POSITION, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_POSITION);
    return result;
}


std::string TraCI_Commands::vehicleGetEdgeID(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_ROAD_ID);

    std::string result = genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROAD_ID, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_ROAD_ID);
    return result;
}


std::string TraCI_Commands::vehicleGetLaneID(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_LANE_ID);

    std::string result = genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_ID, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_LANE_ID);
    return result;
}


uint32_t TraCI_Commands::vehicleGetLaneIndex(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_LANE_INDEX);

    int32_t result = genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_INDEX, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_LANE_INDEX);
    return result;
}


double TraCI_Commands::vehicleGetLanePosition(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_LANEPOSITION);

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANEPOSITION, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_LANEPOSITION);
    return result;
}


std::string TraCI_Commands::vehicleGetTypeID(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_TYPE);

    std::string result = genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TYPE, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_TYPE);
    return result;
}


std::string TraCI_Commands::vehicleGetRouteID(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_ROUTE_ID);

    std::string result = genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROUTE_ID, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_ROUTE_ID);
    return result;
}


std::list<std::string> TraCI_Commands::vehicleGetRoute(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_EDGES);

    std::list<std::string> result = genericGetStringList(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_EDGES, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_EDGES);
    return result;
}


uint32_t TraCI_Commands::vehicleGetRouteIndex(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_ROUTE_INDEX);

    int32_t result = genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x69, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_ROUTE_INDEX);
    return result;
}


std::map<int,bestLanesEntry> TraCI_Commands::vehicleGetBestLanes(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_BEST_LANES);

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

    std::map<int,bestLanesEntry> final;

    for (uint32_t i = 0; i < No; ++i)
    {
        bestLanesEntry *entry = new bestLanesEntry();

        // get lane id
        int8_t sType; buf >> sType;
        std::string laneId; buf >> laneId;
        entry->laneId = laneId;

        // length
        int8_t dType1; buf >> dType1;
        double length; buf >> length;
        entry->length = length;

        // occupation
        int8_t dType2; buf >> dType2;
        double occupation; buf >> occupation;
        entry->occupation = occupation;

        // offset
        int8_t bType; buf >> bType;
        int8_t offset; buf >> offset;
        entry->offset = offset;

        // continuing drive?
        int8_t bType2; buf >> bType2;
        uint8_t continuingDrive; buf >> continuingDrive;
        entry->continuingDrive = continuingDrive;

        // best subsequent lanes
        std::vector<std::string> res;
        int8_t bType3; buf >> bType3;
        uint32_t count; buf >> count;
        for (uint32_t i = 0; i < count; i++)
        {
            std::string id; buf >> id;
            res.push_back(id);
        }
        entry->best = res;

        // add into the map
        final.insert( std::make_pair(i,*entry) );
    }

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_BEST_LANES);

    return final;
}


uint8_t* TraCI_Commands::vehicleGetColor(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_COLOR);

    uint8_t *result = genericGetArrayUnsignedInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_COLOR, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_COLOR);
    return result;
}


uint32_t TraCI_Commands::vehicleGetSignalStatus(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_SIGNALS);

    uint32_t result = genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x5b, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_SIGNALS);
    return result;
}


double TraCI_Commands::vehicleGetLength(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_LENGTH);

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LENGTH, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_LENGTH);
    return result;
}


double TraCI_Commands::vehicleGetMinGap(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_MINGAP);

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_MINGAP, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_MINGAP);
    return result;
}


double TraCI_Commands::vehicleGetMaxAccel(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_ACCEL);

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ACCEL, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_ACCEL);
    return result;
}


double TraCI_Commands::vehicleGetMaxDecel(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_DECEL);

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_DECEL, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_DECEL);
    return result;
}


double TraCI_Commands::vehicleGetTimeGap(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_TAU);

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TAU, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_TAU);
    return result;
}


std::string TraCI_Commands::vehicleGetClass(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_VEHICLECLASS);

    std::string result = genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_VEHICLECLASS, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_VEHICLECLASS);
    return result;
}


std::vector<std::string> TraCI_Commands::vehicleGetLeader(std::string nodeId, double look_ahead_distance)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, VAR_LEADER);

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
    std::vector<std::string> res;

    uint8_t dType; buf >> dType;
    std::string id; buf >> id;
    res.push_back(id);

    uint8_t dType2; buf >> dType2;
    double len; buf >> len;

    // the distance does not include minGap
    // we will add minGap to the len
    len = len + vehicleGetMinGap(nodeId);

    // now convert len to std::string
    std::ostringstream s;
    s << len;
    res.push_back(s.str());

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, VAR_LEADER);

    return res;
}


double TraCI_Commands::vehicleGetCurrentAccel(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, 0x70);

    double result = genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x70, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, 0x70);
    return result;
}


int TraCI_Commands::vehicleGetCarFollowingMode(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, 0x71);

    int result = genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x71, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, 0x71);
    return result;
}


std::string TraCI_Commands::vehicleGetTLID(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, 0x72);

    std::string result = genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x72, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, 0x72);
    return result;
}


char TraCI_Commands::vehicleGetTLLinkStatus(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLE_VARIABLE, 0x73);

    int result = genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x73, RESPONSE_GET_VEHICLE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLE_VARIABLE, 0x73);

    // covert to character
    return (char)result;
}


// #########################
// CMD_SET_VEHICLE_VARIABLE
// #########################

// Let the vehicle stop at the given edge, at the given position and lane.
// The vehicle will stop for the given duration.
void TraCI_Commands::vehicleSetStop(std::string nodeId, std::string edgeId, double stopPos, uint8_t laneId, double waitT, uint8_t flag)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, CMD_STOP);

    uint8_t variableId = CMD_STOP;
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 5;
    uint8_t edgeIdT = TYPE_STRING;
    uint8_t stopPosT = TYPE_DOUBLE;
    uint8_t stopLaneT = TYPE_BYTE;
    uint8_t durationT = TYPE_INTEGER;
    uint32_t duration = waitT * 1000;
    uint8_t flagT = TYPE_BYTE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId
            << variableType << count
            << edgeIdT << edgeId
            << stopPosT << stopPos
            << stopLaneT << laneId
            << durationT << duration
            << flagT << flag);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, CMD_STOP);
}


void TraCI_Commands::vehicleResume(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, CMD_RESUME);

    uint8_t variableId = CMD_RESUME;
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 0;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId
            << variableType << count);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, CMD_RESUME);
}


void TraCI_Commands::vehicleSetSpeed(std::string nodeId, double speed)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, VAR_SPEED);

    uint8_t variableId = VAR_SPEED;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, VAR_SPEED);
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
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, VAR_LANECHANGE_MODE);

    uint8_t variableId = 0xb6;
    uint8_t variableType = TYPE_INTEGER;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << bitset);
    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, VAR_LANECHANGE_MODE);
}


void TraCI_Commands::vehicleChangeLane(std::string nodeId, uint8_t laneId, double duration)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, CMD_CHANGELANE);

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

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, CMD_CHANGELANE);
}


void TraCI_Commands::vehicleSetRoute(std::string id, std::list<std::string> value)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, VAR_ROUTE);

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

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, VAR_ROUTE);
}


void TraCI_Commands::vehicleSetRouteID(std::string nodeId, std::string routeID)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, VAR_ROUTE_ID);

    uint8_t variableId = VAR_ROUTE_ID;
    uint8_t variableType = TYPE_STRING;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << routeID);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, VAR_ROUTE_ID);
}


void TraCI_Commands::vehicleSetColor(std::string nodeId, const RGB color)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, VAR_COLOR);

    TraCIBuffer p;
    p << static_cast<uint8_t>(VAR_COLOR);
    p << nodeId;
    p << static_cast<uint8_t>(TYPE_COLOR) << (uint8_t)color.red << (uint8_t)color.green << (uint8_t)color.blue << (uint8_t)255 /*alpha*/;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, p);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, VAR_COLOR);
}


void TraCI_Commands::vehicleSetClass(std::string nodeId, std::string vClass)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, VAR_VEHICLECLASS);

    uint8_t variableId = VAR_VEHICLECLASS;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << vClass);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, VAR_VEHICLECLASS);
}


void TraCI_Commands::vehicleSetMaxAccel(std::string nodeId, double value)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, VAR_ACCEL);

    uint8_t variableId = VAR_ACCEL;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, VAR_ACCEL);
}


void TraCI_Commands::vehicleSetMaxDecel(std::string nodeId, double value)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, VAR_DECEL);

    uint8_t variableId = VAR_DECEL;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, VAR_DECEL);
}


// this changes vehicle type!
void TraCI_Commands::vehicleSetTimeGap(std::string nodeId, double value)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, VAR_TAU);

    uint8_t variableId = VAR_TAU;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, VAR_TAU);
}


void TraCI_Commands::vehicleAdd(std::string vehicleId, std::string vehicleTypeId, std::string routeId, int32_t depart, double pos, double speed, uint8_t lane)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, ADD);

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
            << lane);          // departure lane

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, ADD);
}


// todo:
//void TraCI_Commands::vehicleAddSimple(std::string vehicleId, std::string vehicleTypeId, std::string routeId, simtime_t emitTime_st, double emitPosition, double emitSpeed, int8_t emitLane)
//{
//    updateTraCIlog("commandStart", , );

//    bool success = false;
//    uint8_t variableId = ADD;
//    uint8_t variableType = TYPE_COMPOUND;
//    int32_t count = 6;
//    int32_t emitTime = (emitTime_st < 0) ? (-1) : (floor(emitTime_st.dbl() * 1000));
//
//    TraCIBuffer buf = connection->queryOptional(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << vehicleId
//            << variableType << count
//            << (uint8_t)TYPE_STRING
//            << vehicleTypeId
//            << (uint8_t)TYPE_STRING
//            << routeId
//            << (uint8_t)TYPE_INTEGER
//            << emitTime
//            << (uint8_t)TYPE_DOUBLE
//            << emitPosition
//            << (uint8_t)TYPE_DOUBLE
//            << emitSpeed
//            << (uint8_t)TYPE_BYTE
//            << emitLane, success);
//
//    ASSERT(buf.eof());

//    updateTraCIlog("commandComplete");
//}


void TraCI_Commands::vehicleRemove(std::string nodeId, uint8_t reason)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, REMOVE);

    uint8_t variableId = REMOVE;
    uint8_t variableType = TYPE_BYTE;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << reason);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, REMOVE);
}


void TraCI_Commands::vehicleSetControllerParameters(std::string nodeId, std::string value)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, 0x15);

    uint8_t variableId = 0x15;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, 0x15);
}


void TraCI_Commands::vehicleSetErrorGap(std::string nodeId, double value)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, 0x20);

    uint8_t variableId = 0x20;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, 0x20);
}


void TraCI_Commands::vehicleSetErrorRelSpeed(std::string nodeId, double value)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, 0x21);

    uint8_t variableId = 0x21;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, 0x21);
}


void TraCI_Commands::vehicleSetDowngradeToACC(std::string nodeId, bool value)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, 0x17);

    uint8_t variableId = 0x17;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << (int)value);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, 0x17);
}


void TraCI_Commands::vehicleSetDebug(std::string nodeId, bool value)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLE_VARIABLE, 0x16);

    uint8_t variableId = 0x16;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << (int)value);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLE_VARIABLE, 0x16);
}



// ################################################################
//                          vehicle type
// ################################################################

// ############################
// CMD_GET_VEHICLETYPE_VARIABLE
// ############################

std::list<std::string> TraCI_Commands::vehicleTypeGetIDList()
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLETYPE_VARIABLE, ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_VEHICLETYPE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLETYPE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLETYPE_VARIABLE, ID_LIST);
    return result;
}


uint32_t TraCI_Commands::vehicleTypeGetIDCount()
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLETYPE_VARIABLE, ID_COUNT);

    uint32_t result = genericGetInt(CMD_GET_VEHICLETYPE_VARIABLE, "", ID_COUNT, RESPONSE_GET_VEHICLETYPE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLETYPE_VARIABLE, ID_COUNT);
    return result;
}


double TraCI_Commands::vehicleTypeGetLength(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLETYPE_VARIABLE, VAR_LENGTH);

    double result = genericGetDouble(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, VAR_LENGTH, RESPONSE_GET_VEHICLETYPE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLETYPE_VARIABLE, VAR_LENGTH);
    return result;
}


double TraCI_Commands::vehicleTypeGetMaxSpeed(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLETYPE_VARIABLE, VAR_MAXSPEED);

    double result = genericGetDouble(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, VAR_MAXSPEED, RESPONSE_GET_VEHICLETYPE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLETYPE_VARIABLE, VAR_MAXSPEED);
    return result;
}


int TraCI_Commands::vehicleTypeGetControllerType(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLETYPE_VARIABLE, 0x02);

    int result = genericGetInt(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, 0x02, RESPONSE_GET_VEHICLETYPE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLETYPE_VARIABLE, 0x02);
    return result;
}


int TraCI_Commands::vehicleTypeGetControllerNumber(std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_GET_VEHICLETYPE_VARIABLE, 0x03);

    int result = genericGetInt(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, 0x03, RESPONSE_GET_VEHICLETYPE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_VEHICLETYPE_VARIABLE, 0x03);
    return result;
}


// ############################
// CMD_SET_VEHICLETYPE_VARIABLE
// ############################

void TraCI_Commands::vehicleTypeSetMaxSpeed(std::string nodeId, double speed)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLETYPE_VARIABLE, VAR_MAXSPEED);

    uint8_t variableId = VAR_MAXSPEED;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLETYPE_VARIABLE, VAR_MAXSPEED);
}


void TraCI_Commands::vehicleTypeSetVint(std::string nodeId, double value)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLETYPE_VARIABLE, 0x22);

    uint8_t variableId = 0x22;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLETYPE_VARIABLE, 0x22);
}


void TraCI_Commands::vehicleTypeSetComfAccel(std::string nodeId, double speed)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLETYPE_VARIABLE, 0x23);

    uint8_t variableId = 0x23;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLETYPE_VARIABLE, 0x23);
}


void TraCI_Commands::vehicleTypeSetComfDecel(std::string nodeId, double speed)
{
    updateTraCIlog("commandStart", CMD_SET_VEHICLETYPE_VARIABLE, 0x24);

    uint8_t variableId = 0x24;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_VEHICLETYPE_VARIABLE, 0x24);
}


// ################################################################
//                              route
// ################################################################

// ######################
// CMD_GET_ROUTE_VARIABLE
// ######################

std::list<std::string> TraCI_Commands::routeGetIDList()
{
    updateTraCIlog("commandStart", CMD_GET_ROUTE_VARIABLE, ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_ROUTE_VARIABLE, "", ID_LIST, RESPONSE_GET_ROUTE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_ROUTE_VARIABLE, ID_LIST);
    return result;
}


uint32_t TraCI_Commands::routeGetIDCount()
{
    updateTraCIlog("commandStart", CMD_GET_ROUTE_VARIABLE, ID_COUNT);

    uint32_t result = genericGetInt(CMD_GET_ROUTE_VARIABLE, "", ID_COUNT, RESPONSE_GET_ROUTE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_ROUTE_VARIABLE, ID_COUNT);
    return result;
}


std::list<std::string> TraCI_Commands::routeGetEdges(std::string routeID)
{
    updateTraCIlog("commandStart", CMD_GET_ROUTE_VARIABLE, VAR_EDGES);

    std::list<std::string> result = genericGetStringList(CMD_GET_ROUTE_VARIABLE, routeID, VAR_EDGES, RESPONSE_GET_ROUTE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_ROUTE_VARIABLE, VAR_EDGES);
    return result;
}


// #######################
// CMD_SET_ROUTE_VARIABLE
// #######################

void TraCI_Commands::routeAdd(std::string name, std::list<std::string> route)
{
    updateTraCIlog("commandStart", CMD_SET_ROUTE_VARIABLE, ADD);

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

    updateTraCIlog("commandComplete", CMD_SET_ROUTE_VARIABLE, ADD);
}


// ################################################################
//                              edge
// ################################################################

// ######################
// CMD_GET_EDGE_VARIABLE
// ######################

std::list<std::string> TraCI_Commands::edgeGetIDList()
{
    updateTraCIlog("commandStart", CMD_GET_EDGE_VARIABLE, ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_EDGE_VARIABLE, "", ID_LIST, RESPONSE_GET_EDGE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_EDGE_VARIABLE, ID_LIST);
    return result;
}


uint32_t TraCI_Commands::edgeGetIDCount()
{
    updateTraCIlog("commandStart", CMD_GET_EDGE_VARIABLE, ID_COUNT);

    uint32_t result = genericGetInt(CMD_GET_EDGE_VARIABLE, "", ID_COUNT, RESPONSE_GET_EDGE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_EDGE_VARIABLE, ID_COUNT);
    return result;
}


double TraCI_Commands::edgeGetMeanTravelTime(std::string Id)
{
    updateTraCIlog("commandStart", CMD_GET_EDGE_VARIABLE, VAR_CURRENT_TRAVELTIME);

    double result = genericGetDouble(CMD_GET_EDGE_VARIABLE, Id, VAR_CURRENT_TRAVELTIME, RESPONSE_GET_EDGE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_EDGE_VARIABLE, VAR_CURRENT_TRAVELTIME);
    return result;
}


uint32_t TraCI_Commands::edgeGetLastStepVehicleNumber(std::string Id)
{
    updateTraCIlog("commandStart", CMD_GET_EDGE_VARIABLE, LAST_STEP_VEHICLE_NUMBER);

    uint32_t result = genericGetInt(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_VEHICLE_NUMBER, RESPONSE_GET_EDGE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_EDGE_VARIABLE, LAST_STEP_VEHICLE_NUMBER);
    return result;
}


std::list<std::string> TraCI_Commands::edgeGetLastStepVehicleIDs(std::string Id)
{
    updateTraCIlog("commandStart", CMD_GET_EDGE_VARIABLE, LAST_STEP_VEHICLE_ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_EDGE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_EDGE_VARIABLE, LAST_STEP_VEHICLE_ID_LIST);
    return result;
}


double TraCI_Commands::edgeGetLastStepMeanVehicleSpeed(std::string Id)
{
    updateTraCIlog("commandStart", CMD_GET_EDGE_VARIABLE, LAST_STEP_MEAN_SPEED);

    double result = genericGetDouble(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_MEAN_SPEED, RESPONSE_GET_EDGE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_EDGE_VARIABLE, LAST_STEP_MEAN_SPEED);
    return result;
}


double TraCI_Commands::edgeGetLastStepMeanVehicleLength(std::string Id)
{
    updateTraCIlog("commandStart", CMD_GET_EDGE_VARIABLE, LAST_STEP_LENGTH);

    double result = genericGetDouble(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_LENGTH, RESPONSE_GET_EDGE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_EDGE_VARIABLE, LAST_STEP_LENGTH);
    return result;
}


std::list<std::string> TraCI_Commands::edgeGetLastStepPersonIDs(std::string Id)
{
    updateTraCIlog("commandStart", CMD_GET_EDGE_VARIABLE, LAST_STEP_PERSON_ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_EDGE_VARIABLE, Id, 0x1a, RESPONSE_GET_EDGE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_EDGE_VARIABLE, LAST_STEP_PERSON_ID_LIST);
    return result;
}


// ######################
// CMD_SET_EDGE_VARIABLE
// ######################

void TraCI_Commands::edgeSetGlobalTravelTime(std::string edgeId, int32_t beginT, int32_t endT, double value)
{
    updateTraCIlog("commandStart", CMD_SET_EDGE_VARIABLE, VAR_EDGE_TRAVELTIME);

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

    updateTraCIlog("commandComplete", CMD_SET_EDGE_VARIABLE, VAR_EDGE_TRAVELTIME);
}


// ################################################################
//                              lane
// ################################################################

// #####################
// CMD_GET_LANE_VARIABLE
// #####################

// gets a list of all lanes in the network
std::list<std::string> TraCI_Commands::laneGetIDList()
{
    updateTraCIlog("commandStart", CMD_GET_LANE_VARIABLE, ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_LANE_VARIABLE, "", ID_LIST, RESPONSE_GET_LANE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_LANE_VARIABLE, ID_LIST);
    return result;
}


uint32_t TraCI_Commands::laneGetIDCount()
{
    updateTraCIlog("commandStart", CMD_GET_LANE_VARIABLE, ID_COUNT);

    uint32_t result = genericGetInt(CMD_GET_LANE_VARIABLE, "", ID_COUNT, RESPONSE_GET_LANE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_LANE_VARIABLE, ID_COUNT);
    return result;
}


uint8_t TraCI_Commands::laneGetLinkNumber(std::string laneId)
{
    updateTraCIlog("commandStart", CMD_GET_LANE_VARIABLE, LANE_LINK_NUMBER);

    uint8_t result = genericGetUnsignedByte(CMD_GET_LANE_VARIABLE, laneId, LANE_LINK_NUMBER, RESPONSE_GET_LANE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_LANE_VARIABLE, LANE_LINK_NUMBER);
    return result;
}


std::map<int,linkEntry> TraCI_Commands::laneGetLinks(std::string laneId)
{
    updateTraCIlog("commandStart", CMD_GET_LANE_VARIABLE, LANE_LINKS);

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

    std::map<int,linkEntry> final;

    for (uint32_t i = 0; i < No; ++i)
    {
        linkEntry *entry = new linkEntry();

        // get consecutive1
        int8_t sType; buf >> sType;
        std::string consecutive1; buf >> consecutive1;
        entry->consecutive1 = consecutive1;

        // get consecutive2
        int8_t sType2; buf >> sType2;
        std::string consecutive2; buf >> consecutive2;
        entry->consecutive2 = consecutive2;

        // priority
        int8_t bType1; buf >> bType1;
        uint8_t priority; buf >> priority;
        entry->priority = priority;

        // opened
        int8_t bType2; buf >> bType2;
        uint8_t opened; buf >> opened;
        entry->opened = opened;

        // approachingFoe
        int8_t bType3; buf >> bType3;
        uint8_t approachingFoe; buf >> approachingFoe;
        entry->approachingFoe = approachingFoe;

        // get state
        int8_t sType3; buf >> sType3;
        std::string state; buf >> state;
        entry->state = state;

        // get direction
        int8_t sType4; buf >> sType4;
        std::string direction; buf >> direction;
        entry->direction = direction;

        // length
        int8_t dType1; buf >> dType1;
        double length; buf >> length;
        entry->length = length;

        // add into the map
        final.insert( std::make_pair(i,*entry) );
    }

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_GET_LANE_VARIABLE, LANE_LINKS);

    return final;
}


std::list<std::string> TraCI_Commands::laneGetAllowedClasses(std::string laneId)
{
    updateTraCIlog("commandStart", CMD_GET_LANE_VARIABLE, LANE_ALLOWED);

    std::list<std::string> result = genericGetStringList(CMD_GET_LANE_VARIABLE, laneId, LANE_ALLOWED, RESPONSE_GET_LANE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_LANE_VARIABLE, LANE_ALLOWED);
    return result;
}


std::string TraCI_Commands::laneGetEdgeID(std::string laneId)
{
    updateTraCIlog("commandStart", CMD_GET_LANE_VARIABLE, LANE_EDGE_ID);

    std::string result = genericGetString(CMD_GET_LANE_VARIABLE, laneId, LANE_EDGE_ID, RESPONSE_GET_LANE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_LANE_VARIABLE, LANE_EDGE_ID);
    return result;
}


double TraCI_Commands::laneGetLength(std::string laneId)
{
    updateTraCIlog("commandStart", CMD_GET_LANE_VARIABLE, VAR_LENGTH);

    double result = genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_LENGTH, RESPONSE_GET_LANE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_LANE_VARIABLE, VAR_LENGTH);
    return result;
}


double TraCI_Commands::laneGetMaxSpeed(std::string laneId)
{
    updateTraCIlog("commandStart", CMD_GET_LANE_VARIABLE, VAR_MAXSPEED);

    double result = genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_MAXSPEED, RESPONSE_GET_LANE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_LANE_VARIABLE, VAR_MAXSPEED);
    return result;
}


uint32_t TraCI_Commands::laneGetLastStepVehicleNumber(std::string laneId)
{
    updateTraCIlog("commandStart", CMD_GET_LANE_VARIABLE, LAST_STEP_VEHICLE_NUMBER);

    uint32_t result = genericGetInt(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_VEHICLE_NUMBER, RESPONSE_GET_LANE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_LANE_VARIABLE, LAST_STEP_VEHICLE_NUMBER);
    return result;
}


std::list<std::string> TraCI_Commands::laneGetLastStepVehicleIDs(std::string laneId)
{
    updateTraCIlog("commandStart", CMD_GET_LANE_VARIABLE, LAST_STEP_VEHICLE_ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_LANE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_LANE_VARIABLE, LAST_STEP_VEHICLE_ID_LIST);
    return result;
}


double TraCI_Commands::laneGetLastStepMeanVehicleSpeed(std::string laneId)
{
    updateTraCIlog("commandStart", CMD_GET_LANE_VARIABLE, LAST_STEP_MEAN_SPEED);

    double result = genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_MEAN_SPEED, RESPONSE_GET_LANE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_LANE_VARIABLE, LAST_STEP_MEAN_SPEED);
    return result;
}


double TraCI_Commands::laneGetLastStepMeanVehicleLength(std::string laneId)
{
    updateTraCIlog("commandStart", CMD_GET_LANE_VARIABLE, LAST_STEP_LENGTH);

    double result = genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_LENGTH, RESPONSE_GET_LANE_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_LANE_VARIABLE, LAST_STEP_LENGTH);
    return result;
}


// ######################
// CMD_SET_LANE_VARIABLE
// ######################

void TraCI_Commands::laneSetMaxSpeed(std::string laneId, double value)
{
    updateTraCIlog("commandStart", CMD_SET_LANE_VARIABLE, VAR_MAXSPEED);

    uint8_t variableId = VAR_MAXSPEED;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_LANE_VARIABLE, TraCIBuffer() << variableId << laneId << variableType << value);
    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_LANE_VARIABLE, VAR_MAXSPEED);
}



// ################################################################
//                 loop detector (E1-Detectors)
// ################################################################

// ###############################
// CMD_GET_INDUCTIONLOOP_VARIABLE
// ###############################

std::list<std::string> TraCI_Commands::LDGetIDList()
{
    updateTraCIlog("commandStart", CMD_GET_INDUCTIONLOOP_VARIABLE, ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_INDUCTIONLOOP_VARIABLE, "", ID_LIST, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_INDUCTIONLOOP_VARIABLE, ID_LIST);
    return result;
}


uint32_t TraCI_Commands::LDGetIDCount(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_INDUCTIONLOOP_VARIABLE, ID_COUNT);

    uint32_t result = genericGetInt(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, ID_COUNT, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_INDUCTIONLOOP_VARIABLE, ID_COUNT);
    return result;
}


std::string TraCI_Commands::LDGetLaneID(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_INDUCTIONLOOP_VARIABLE, VAR_LANE_ID);

    std::string result = genericGetString(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, VAR_LANE_ID, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_INDUCTIONLOOP_VARIABLE, VAR_LANE_ID);
    return result;
}


double TraCI_Commands::LDGetPosition(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_INDUCTIONLOOP_VARIABLE, VAR_POSITION);

    double result = genericGetDouble(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, VAR_POSITION, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_INDUCTIONLOOP_VARIABLE, VAR_POSITION);
    return result;
}


uint32_t TraCI_Commands::LDGetLastStepVehicleNumber(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_VEHICLE_NUMBER);

    uint32_t result = genericGetInt(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, LAST_STEP_VEHICLE_NUMBER, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_VEHICLE_NUMBER);
    return result;
}


std::list<std::string> TraCI_Commands::LDGetLastStepVehicleIDs(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_VEHICLE_ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_VEHICLE_ID_LIST);
    return result;
}


double TraCI_Commands::LDGetLastStepMeanVehicleSpeed(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_MEAN_SPEED);

    double result = genericGetDouble(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, LAST_STEP_MEAN_SPEED, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_MEAN_SPEED);
    return result;
}


double TraCI_Commands::LDGetElapsedTimeLastDetection(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_TIME_SINCE_DETECTION);

    double result = genericGetDouble(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, LAST_STEP_TIME_SINCE_DETECTION, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_TIME_SINCE_DETECTION);
    return result;
}


std::vector<std::string> TraCI_Commands::LDGetLastStepVehicleData(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_VEHICLE_DATA);

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
    std::vector<std::string> res;
    std::ostringstream strs;

    // number of information packets (in 'No' variable)
    uint8_t typeI; buf >> typeI;
    uint32_t No; buf >> No;

    for (uint32_t i = 1; i <= No; ++i)
    {
        // get vehicle id
        uint8_t typeS1; buf >> typeS1;
        std::string vId; buf >> vId;
        res.push_back(vId);

        // get vehicle length
        uint8_t dType2; buf >> dType2;
        double len; buf >> len;
        strs.str("");
        strs << len;
        res.push_back( strs.str() );

        // entryTime
        uint8_t dType3; buf >> dType3;
        double entryTime; buf >> entryTime;
        strs.str("");
        strs << entryTime;
        res.push_back( strs.str() );

        // leaveTime
        uint8_t dType4; buf >> dType4;
        double leaveTime; buf >> leaveTime;
        strs.str("");
        strs << leaveTime;
        res.push_back( strs.str() );

        // vehicle type
        uint8_t dType5; buf >> dType5;
        std::string vehicleTypeID; buf >> vehicleTypeID;
        strs.str("");
        strs << vehicleTypeID;
        res.push_back( strs.str() );
    }

    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_VEHICLE_DATA);

    return res;
}


double TraCI_Commands::LDGetLastStepOccupancy(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_OCCUPANCY);

    double result = genericGetDouble(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x13, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_INDUCTIONLOOP_VARIABLE, LAST_STEP_OCCUPANCY);

    return result;
}


// ################################################################
//                lane area detector (E2-Detectors)
// ################################################################

// ###############################
// CMD_GET_AREAL_DETECTOR_VARIABLE
// ###############################

std::list<std::string> TraCI_Commands::LADGetIDList()
{
    updateTraCIlog("commandStart", CMD_GET_AREAL_DETECTOR_VARIABLE, ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_AREAL_DETECTOR_VARIABLE, "", ID_LIST, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_AREAL_DETECTOR_VARIABLE, ID_LIST);
    return result;
}


uint32_t TraCI_Commands::LADGetIDCount(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_AREAL_DETECTOR_VARIABLE, ID_COUNT);

    uint32_t result = genericGetInt(CMD_GET_AREAL_DETECTOR_VARIABLE, loopId, ID_COUNT, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_AREAL_DETECTOR_VARIABLE, ID_COUNT);
    return result;
}


std::string TraCI_Commands::LADGetLaneID(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_AREAL_DETECTOR_VARIABLE, VAR_LANE_ID);

    std::string result = genericGetString(CMD_GET_AREAL_DETECTOR_VARIABLE, loopId, VAR_LANE_ID, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_AREAL_DETECTOR_VARIABLE, VAR_LANE_ID);
    return result;
}


uint32_t TraCI_Commands::LADGetLastStepVehicleNumber(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_VEHICLE_NUMBER);

    uint32_t result = genericGetInt(CMD_GET_AREAL_DETECTOR_VARIABLE, loopId, LAST_STEP_VEHICLE_NUMBER, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_VEHICLE_NUMBER);
    return result;
}


std::list<std::string> TraCI_Commands::LADGetLastStepVehicleIDs(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_VEHICLE_ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_AREAL_DETECTOR_VARIABLE, loopId, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_VEHICLE_ID_LIST);
    return result;
}


double TraCI_Commands::LADGetLastStepMeanVehicleSpeed(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_MEAN_SPEED);

    double result = genericGetDouble(CMD_GET_AREAL_DETECTOR_VARIABLE, loopId, LAST_STEP_MEAN_SPEED, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_MEAN_SPEED);
    return result;
}


uint32_t TraCI_Commands::LADGetLastStepVehicleHaltingNumber(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_VEHICLE_HALTING_NUMBER);

    double result = genericGetInt(CMD_GET_AREAL_DETECTOR_VARIABLE, loopId, LAST_STEP_VEHICLE_HALTING_NUMBER, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_AREAL_DETECTOR_VARIABLE, LAST_STEP_VEHICLE_HALTING_NUMBER);
    return result;
}

double TraCI_Commands::LADGetLastStepJamLengthInMeter(std::string loopId)
{
    updateTraCIlog("commandStart", CMD_GET_AREAL_DETECTOR_VARIABLE, JAM_LENGTH_METERS);

    double result = genericGetDouble(CMD_GET_AREAL_DETECTOR_VARIABLE, loopId, JAM_LENGTH_METERS, RESPONSE_GET_AREAL_DETECTOR_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_AREAL_DETECTOR_VARIABLE, JAM_LENGTH_METERS);
    return result;
}


// ################################################################
//                          traffic light
// ################################################################

// ###################
// CMD_GET_TL_VARIABLE
// ###################

std::list<std::string> TraCI_Commands::TLGetIDList()
{
    updateTraCIlog("commandStart", CMD_GET_TL_VARIABLE, ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_TL_VARIABLE, "", ID_LIST, RESPONSE_GET_TL_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_TL_VARIABLE, ID_LIST);
    return result;
}


uint32_t TraCI_Commands::TLGetIDCount()
{
    updateTraCIlog("commandStart", CMD_GET_TL_VARIABLE, ID_COUNT);

    uint32_t result = genericGetInt(CMD_GET_TL_VARIABLE, "", ID_COUNT, RESPONSE_GET_TL_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_TL_VARIABLE, ID_COUNT);
    return result;
}


std::list<std::string> TraCI_Commands::TLGetControlledLanes(std::string TLid)
{
    updateTraCIlog("commandStart", CMD_GET_TL_VARIABLE, TL_CONTROLLED_LANES);

    std::list<std::string> result = genericGetStringList(CMD_GET_TL_VARIABLE, TLid, TL_CONTROLLED_LANES, RESPONSE_GET_TL_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_TL_VARIABLE, TL_CONTROLLED_LANES);
    return result;
}


std::map<int,std::vector<std::string>> TraCI_Commands::TLGetControlledLinks(std::string TLid)
{
    updateTraCIlog("commandStart", CMD_GET_TL_VARIABLE, TL_CONTROLLED_LINKS);

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

    updateTraCIlog("commandComplete", CMD_GET_TL_VARIABLE, TL_CONTROLLED_LINKS);

    return myMap;
}


std::string TraCI_Commands::TLGetProgram(std::string TLid)
{
    updateTraCIlog("commandStart", CMD_GET_TL_VARIABLE, TL_CURRENT_PROGRAM);

    std::string result = genericGetString(CMD_GET_TL_VARIABLE, TLid, TL_CURRENT_PROGRAM, RESPONSE_GET_TL_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_TL_VARIABLE, TL_CURRENT_PROGRAM);
    return result;
}


uint32_t TraCI_Commands::TLGetPhase(std::string TLid)
{
    updateTraCIlog("commandStart", CMD_GET_TL_VARIABLE, TL_CURRENT_PHASE);

    uint32_t result = genericGetInt(CMD_GET_TL_VARIABLE, TLid, TL_CURRENT_PHASE, RESPONSE_GET_TL_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_TL_VARIABLE, TL_CURRENT_PHASE);
    return result;
}


std::string TraCI_Commands::TLGetState(std::string TLid)
{
    updateTraCIlog("commandStart", CMD_GET_TL_VARIABLE, TL_RED_YELLOW_GREEN_STATE);

    std::string result = genericGetString(CMD_GET_TL_VARIABLE, TLid, TL_RED_YELLOW_GREEN_STATE, RESPONSE_GET_TL_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_TL_VARIABLE, TL_RED_YELLOW_GREEN_STATE);
    return result;
}


uint32_t TraCI_Commands::TLGetPhaseDuration(std::string TLid)
{
    updateTraCIlog("commandStart", CMD_GET_TL_VARIABLE, TL_PHASE_DURATION);

    uint32_t result = genericGetInt(CMD_GET_TL_VARIABLE, TLid, TL_PHASE_DURATION, RESPONSE_GET_TL_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_TL_VARIABLE, TL_PHASE_DURATION);
    return result;
}


uint32_t TraCI_Commands::TLGetNextSwitchTime(std::string TLid)
{
    updateTraCIlog("commandStart", CMD_GET_TL_VARIABLE, TL_NEXT_SWITCH);

    uint32_t result = genericGetInt(CMD_GET_TL_VARIABLE, TLid, TL_NEXT_SWITCH, RESPONSE_GET_TL_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_TL_VARIABLE, TL_NEXT_SWITCH);
    return result;
}


// ###################
// CMD_SET_TL_VARIABLE
// ###################

void TraCI_Commands::TLSetProgram(std::string TLid, std::string value)
{
    updateTraCIlog("commandStart", CMD_SET_TL_VARIABLE, TL_PROGRAM);

    uint8_t variableId = TL_PROGRAM;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_TL_VARIABLE, TL_PROGRAM);
}


void TraCI_Commands::TLSetPhaseIndex(std::string TLid, int value)
{
    updateTraCIlog("commandStart", CMD_SET_TL_VARIABLE, TL_PHASE_INDEX);

    uint8_t variableId = TL_PHASE_INDEX;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_TL_VARIABLE, TL_PHASE_INDEX);
}


void TraCI_Commands::TLSetPhaseDuration(std::string TLid, int value)
{
    updateTraCIlog("commandStart", CMD_SET_TL_VARIABLE, TL_PHASE_DURATION);

    uint8_t variableId = TL_PHASE_DURATION;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_TL_VARIABLE, TL_PHASE_DURATION);
}


void TraCI_Commands::TLSetState(std::string TLid, std::string value)
{
    updateTraCIlog("commandStart", CMD_SET_TL_VARIABLE, TL_RED_YELLOW_GREEN_STATE);

    uint8_t variableId = TL_RED_YELLOW_GREEN_STATE;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_TL_VARIABLE, TL_RED_YELLOW_GREEN_STATE);
}


// ################################################################
//                             Junction
// ################################################################

// #########################
// CMD_GET_JUNCTION_VARIABLE
// #########################

std::list<std::string> TraCI_Commands::junctionGetIDList()
{
    updateTraCIlog("commandStart", CMD_GET_JUNCTION_VARIABLE, ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_JUNCTION_VARIABLE, "", ID_LIST, RESPONSE_GET_JUNCTION_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_JUNCTION_VARIABLE, ID_LIST);
    return result;
}


uint32_t TraCI_Commands::junctionGetIDCount()
{
    updateTraCIlog("commandStart", CMD_GET_JUNCTION_VARIABLE, ID_COUNT);

    uint32_t result = genericGetInt(CMD_GET_JUNCTION_VARIABLE, "", ID_COUNT, RESPONSE_GET_JUNCTION_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_JUNCTION_VARIABLE, ID_COUNT);
    return result;
}


Coord TraCI_Commands::junctionGetPosition(std::string id)
{
    updateTraCIlog("commandStart", CMD_GET_JUNCTION_VARIABLE, VAR_POSITION);

    Coord result = genericGetCoordv2(CMD_GET_JUNCTION_VARIABLE, id, VAR_POSITION, RESPONSE_GET_JUNCTION_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_JUNCTION_VARIABLE, VAR_POSITION);
    return result;
}


// ################################################################
//                               GUI
// ################################################################

// #####################
// CMD_GET_GUI_VARIABLE
// #####################

Coord TraCI_Commands::GUIGetOffset(std::string viewID)
{
    updateTraCIlog("commandStart", CMD_GET_GUI_VARIABLE, VAR_VIEW_OFFSET);

    Coord result = genericGetCoordv2(CMD_GET_GUI_VARIABLE, viewID, VAR_VIEW_OFFSET, RESPONSE_GET_GUI_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_GUI_VARIABLE, VAR_VIEW_OFFSET);
    return result;
}


std::vector<double> TraCI_Commands::GUIGetBoundry(std::string viewID)
{
    updateTraCIlog("commandStart", CMD_GET_GUI_VARIABLE, VAR_VIEW_BOUNDARY);

    std::vector<double> result = genericGetBoundingBox(CMD_GET_GUI_VARIABLE, viewID, VAR_VIEW_BOUNDARY, RESPONSE_GET_GUI_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_GUI_VARIABLE, VAR_VIEW_BOUNDARY);
    return result;
}


// #####################
// CMD_SET_GUI_VARIABLE
// #####################

void TraCI_Commands::GUISetZoom(std::string viewID, double value)
{
    updateTraCIlog("commandStart", CMD_SET_GUI_VARIABLE, VAR_VIEW_ZOOM);

    uint8_t variableId = VAR_VIEW_ZOOM;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << value);
    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_GUI_VARIABLE, VAR_VIEW_ZOOM);
}


void TraCI_Commands::GUISetOffset(std::string viewID, double x, double y)
{
    updateTraCIlog("commandStart", CMD_SET_GUI_VARIABLE, VAR_VIEW_OFFSET);

    uint8_t variableId = VAR_VIEW_OFFSET;
    uint8_t variableType = POSITION_2D;

    TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << x << y);
    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_GUI_VARIABLE, VAR_VIEW_OFFSET);
}


// very slow!
void TraCI_Commands::GUISetTrackVehicle(std::string viewID, std::string nodeId)
{
    updateTraCIlog("commandStart", CMD_SET_GUI_VARIABLE, VAR_TRACK_VEHICLE);

    uint8_t variableId = VAR_TRACK_VEHICLE;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << nodeId);
    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_GUI_VARIABLE, VAR_TRACK_VEHICLE);
}


// ################################################################
//                               polygon
// ################################################################

// ########################
// CMD_GET_POLYGON_VARIABLE
// ########################

std::list<std::string> TraCI_Commands::polygonGetIDList()
{
    updateTraCIlog("commandStart", CMD_GET_POLYGON_VARIABLE, ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_POLYGON_VARIABLE, "", ID_LIST, RESPONSE_GET_POLYGON_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_POLYGON_VARIABLE, ID_LIST);
    return result;
}


uint32_t TraCI_Commands::polygonGetIDCount()
{
    updateTraCIlog("commandStart", CMD_GET_POLYGON_VARIABLE, ID_COUNT);

    uint32_t result = genericGetInt(CMD_GET_POLYGON_VARIABLE, "", ID_COUNT, RESPONSE_GET_POLYGON_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_POLYGON_VARIABLE, ID_COUNT);
    return result;
}


std::list<Coord> TraCI_Commands::polygonGetShape(std::string polyId)
{
    updateTraCIlog("commandStart", CMD_GET_POLYGON_VARIABLE, VAR_SHAPE);

    std::list<Coord> result = genericGetCoordList(CMD_GET_POLYGON_VARIABLE, polyId, VAR_SHAPE, RESPONSE_GET_POLYGON_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_POLYGON_VARIABLE, VAR_SHAPE);
    return result;
}


std::string TraCI_Commands::polygonGetTypeID(std::string polyId)
{
    updateTraCIlog("commandStart", CMD_GET_POLYGON_VARIABLE, VAR_TYPE);

    std::string result = genericGetString(CMD_GET_POLYGON_VARIABLE, polyId, VAR_TYPE, RESPONSE_GET_POLYGON_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_POLYGON_VARIABLE, VAR_TYPE);
    return result;
}


// ########################
// CMD_SET_POLYGON_VARIABLE
// ########################

void TraCI_Commands::polygonAddTraCI(std::string polyId, std::string polyType, const RGB color, bool filled, int32_t layer, const std::list<TraCICoord>& points)
{
    updateTraCIlog("commandStart", CMD_SET_POLYGON_VARIABLE, ADD);

    TraCIBuffer p;

    p << static_cast<uint8_t>(ADD) << polyId;
    p << static_cast<uint8_t>(TYPE_COMPOUND) << static_cast<int32_t>(5);
    p << static_cast<uint8_t>(TYPE_STRING) << polyType;
    p << static_cast<uint8_t>(TYPE_COLOR) << (uint8_t)color.red << (uint8_t)color.green << (uint8_t)color.blue << (uint8_t)255 /*alpha*/;
    p << static_cast<uint8_t>(TYPE_UBYTE) << static_cast<uint8_t>(filled);
    p << static_cast<uint8_t>(TYPE_INTEGER) << layer;
    p << static_cast<uint8_t>(TYPE_POLYGON) << static_cast<uint8_t>(points.size());

    for (std::list<TraCICoord>::const_iterator i = points.begin(); i != points.end(); ++i)
    {
        const TraCICoord& pos = *i;
        p << static_cast<double>(pos.x) << static_cast<double>(pos.y);
    }

    TraCIBuffer buf = connection->query(CMD_SET_POLYGON_VARIABLE, p);
    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_POLYGON_VARIABLE, ADD);
}


void TraCI_Commands::polygonAdd(std::string polyId, std::string polyType, const RGB color, bool filled, int32_t layer, const std::list<Coord>& points)
{
    updateTraCIlog("commandStart", CMD_SET_POLYGON_VARIABLE, ADD);

    TraCIBuffer p;

    p << static_cast<uint8_t>(ADD) << polyId;
    p << static_cast<uint8_t>(TYPE_COMPOUND) << static_cast<int32_t>(5);
    p << static_cast<uint8_t>(TYPE_STRING) << polyType;
    p << static_cast<uint8_t>(TYPE_COLOR) << (uint8_t)color.red << (uint8_t)color.green << (uint8_t)color.blue << (uint8_t)255 /*alpha*/;
    p << static_cast<uint8_t>(TYPE_UBYTE) << static_cast<uint8_t>(filled);
    p << static_cast<uint8_t>(TYPE_INTEGER) << layer;
    p << static_cast<uint8_t>(TYPE_POLYGON) << static_cast<uint8_t>(points.size());

    for (std::list<Coord>::const_iterator i = points.begin(); i != points.end(); ++i)
    {
        const Coord& pos = *i;
        p << static_cast<double>(pos.x) << static_cast<double>(pos.y);
    }

    TraCIBuffer buf = connection->query(CMD_SET_POLYGON_VARIABLE, p);
    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_POLYGON_VARIABLE, ADD);
}


void TraCI_Commands::polygonSetFilled(std::string polyId, uint8_t filled)
{
    updateTraCIlog("commandStart", CMD_SET_POLYGON_VARIABLE, VAR_FILL);

    uint8_t variableId = VAR_FILL;
    uint8_t variableType = TYPE_UBYTE;

    TraCIBuffer buf = connection->query(CMD_SET_POLYGON_VARIABLE, TraCIBuffer() << variableId << polyId << variableType << filled);
    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_POLYGON_VARIABLE, VAR_FILL);
}


// ################################################################
//                               POI
// ################################################################

void TraCI_Commands::addPoi(std::string poiId, std::string poiType, const RGB color, int32_t layer, const Coord& pos_)
{
    updateTraCIlog("commandStart", CMD_SET_POI_VARIABLE, ADD);

    TraCIBuffer p;

    TraCICoord pos = omnet2traci(pos_);
    p << static_cast<uint8_t>(ADD) << poiId;
    p << static_cast<uint8_t>(TYPE_COMPOUND) << static_cast<int32_t>(4);
    p << static_cast<uint8_t>(TYPE_STRING) << poiType;
    p << static_cast<uint8_t>(TYPE_COLOR) << (uint8_t)color.red << (uint8_t)color.green << (uint8_t)color.blue << (uint8_t)255 /*alpha*/;
    p << static_cast<uint8_t>(TYPE_INTEGER) << layer;
    p << pos;

    TraCIBuffer buf = connection->query(CMD_SET_POI_VARIABLE, p);
    ASSERT(buf.eof());

    updateTraCIlog("commandComplete", CMD_SET_POI_VARIABLE, ADD);
}


// ################################################################
//                               person
// ################################################################

// ##############
// CMD_GET_PERSON
// ##############

std::list<std::string> TraCI_Commands::personGetIDList()
{
    updateTraCIlog("commandStart", CMD_GET_PERSON_VARIABLE, ID_LIST);

    std::list<std::string> result = genericGetStringList(CMD_GET_PERSON_VARIABLE, "", ID_LIST, RESPONSE_GET_PERSON_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_PERSON_VARIABLE, ID_LIST);
    return result;
}


uint32_t TraCI_Commands::personGetIDCount()
{
    updateTraCIlog("commandStart", CMD_GET_PERSON_VARIABLE, ID_COUNT);

    uint32_t result = genericGetInt(CMD_GET_PERSON_VARIABLE, "", ID_COUNT, RESPONSE_GET_PERSON_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_PERSON_VARIABLE, ID_COUNT);
    return result;
}


std::string TraCI_Commands::personGetTypeID(std::string pId)
{
    updateTraCIlog("commandStart", CMD_GET_PERSON_VARIABLE, VAR_TYPE);

    std::string result = genericGetString(CMD_GET_PERSON_VARIABLE, pId, VAR_TYPE, RESPONSE_GET_PERSON_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_PERSON_VARIABLE, VAR_TYPE);
    return result;
}


Coord TraCI_Commands::personGetPosition(std::string pId)
{
    updateTraCIlog("commandStart", CMD_GET_PERSON_VARIABLE, VAR_POSITION);

    Coord result = genericGetCoordv2(CMD_GET_PERSON_VARIABLE, pId, VAR_POSITION, RESPONSE_GET_PERSON_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_PERSON_VARIABLE, VAR_POSITION);
    return result;
}


std::string TraCI_Commands::personGetEdgeID(std::string pId)
{
    updateTraCIlog("commandStart", CMD_GET_PERSON_VARIABLE, VAR_ROAD_ID);

    std::string result = genericGetString(CMD_GET_PERSON_VARIABLE, pId, VAR_ROAD_ID, RESPONSE_GET_PERSON_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_PERSON_VARIABLE, VAR_ROAD_ID);
    return result;
}


double TraCI_Commands::personGetEdgePosition(std::string pId)
{
    updateTraCIlog("commandStart", CMD_GET_PERSON_VARIABLE, VAR_LANEPOSITION);

    double result = genericGetDouble(CMD_GET_PERSON_VARIABLE, pId, VAR_LANEPOSITION, RESPONSE_GET_PERSON_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_PERSON_VARIABLE, VAR_LANEPOSITION);
    return result;
}


double TraCI_Commands::personGetSpeed(std::string pId)
{
    updateTraCIlog("commandStart", CMD_GET_PERSON_VARIABLE, VAR_SPEED);

    double result = genericGetDouble(CMD_GET_PERSON_VARIABLE, pId, VAR_SPEED, RESPONSE_GET_PERSON_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_PERSON_VARIABLE, VAR_SPEED);
    return result;
}


std::string TraCI_Commands::personGetNextEdge(std::string pId)
{
    updateTraCIlog("commandStart", CMD_GET_PERSON_VARIABLE, VAR_NEXT_EDGE);

    std::string result = genericGetString(CMD_GET_PERSON_VARIABLE, pId, VAR_NEXT_EDGE, RESPONSE_GET_PERSON_VARIABLE);

    updateTraCIlog("commandComplete", CMD_GET_PERSON_VARIABLE, VAR_NEXT_EDGE);
    return result;
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


std::list<std::string> TraCI_Commands::genericGetStringList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_STRINGLIST;
    std::list<std::string> res;

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


Coord TraCI_Commands::genericGetCoord(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
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

    return traci2omnet(TraCICoord(x, y));
}


std::list<Coord> TraCI_Commands::genericGetCoordList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_POLYGON;
    std::list<Coord> res;

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
        res.push_back(traci2omnet(TraCICoord(x, y)));
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


// same as genericGetCoordv, but no conversion to omnet++ coordinates at the end
Coord TraCI_Commands::genericGetCoordv2(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
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

    // now we start getting real data that we are looking for
    buf >> x;
    buf >> y;

    ASSERT(buf.eof());

    return Coord(x, y);
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


uint8_t* TraCI_Commands::genericGetArrayUnsignedInt(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_COLOR;
    uint8_t* color = new uint8_t[4]; // RGBA

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

    // now we start getting real data that we are looking for
    buf >> color[0];
    buf >> color[1];
    buf >> color[2];
    buf >> color[3];

    ASSERT(buf.eof());

    return color;
}


// ################################################################
//                      coordinate conversion
// ################################################################

Coord TraCI_Commands::traci2omnet(TraCICoord coord) const
{
    return Coord(coord.x - netbounds1.x + margin, (netbounds2.y - netbounds1.y) - (coord.y - netbounds1.y) + margin);
}


TraCICoord TraCI_Commands::omnet2traci(Coord coord) const
{
    return TraCICoord(coord.x + netbounds1.x - margin, (netbounds2.y - netbounds1.y) - (coord.y - netbounds1.y) + margin);
}


double TraCI_Commands::traci2omnetAngle(double angle) const
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


double TraCI_Commands::omnet2traciAngle(double angle) const
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

std::string TraCI_Commands::getSUMOFullDir()
{
    boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
    std::string SUMOconfig = par("SUMOconfig").stringValue();
    boost::filesystem::path SUMOconfigFullPath = VENTOS_FullPath / SUMOconfig;

    // get the directory
    boost::filesystem::path dir = SUMOconfigFullPath.parent_path();

    if( !boost::filesystem::exists( dir ) )
        error("SUMO directory is not found in %s", dir.string().c_str());

    return dir.string();
}


std::string  TraCI_Commands::getSUMOConfigFullPath()
{
    boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
    std::string SUMOconfig = par("SUMOconfig").stringValue();
    boost::filesystem::path SUMOconfigFullPath = VENTOS_FullPath / SUMOconfig;

    if( !boost::filesystem::exists(SUMOconfigFullPath) || !boost::filesystem::is_regular_file(SUMOconfigFullPath) )
        error("SUMO configure file is not found in %s", SUMOconfigFullPath.string().c_str());

    return SUMOconfigFullPath.string();
}

// ################################################################
//               logging TraCI commands exchange
// ################################################################

void TraCI_Commands::updateTraCIlog(std::string state, uint8_t commandGroupId, uint8_t commandId)
{
    // check if logging is enabled
    bool logEnabled = par("logTraCIcommands").boolValue();
    if(!logEnabled)
        return;

    if(state == "commandStart")
    {
        Htime_t t = std::chrono::high_resolution_clock::now();

        TraCIcommandEntry *entry = new TraCIcommandEntry(simTime().dbl(), t, t, commandGroupId, commandId);
        exchangedTraCIcommands.push_back(*entry);
    }
    else if(state == "commandComplete")
    {
        // do not include vector search in command finish time
        Htime_t t = std::chrono::high_resolution_clock::now();

        bool found = false;
        std::vector<TraCIcommandEntry>::reverse_iterator it;
        for (it = exchangedTraCIcommands.rbegin(); it != exchangedTraCIcommands.rend(); ++it)
        {
            if(it->commandGroupId == commandGroupId && it->commandId == commandId)
            {
                found = true;
                break;
            }
        }

        if(!found)
            error("pair (%x, %x) is not found in exchangedTraCIcommands \n", commandGroupId, commandId);

        it->completeAt = t;
    }
}


void TraCI_Commands::TraCIexchangeToFile()
{
    if(exchangedTraCIcommands.empty())
        return;

    boost::filesystem::path filePath;

    if(ev.isGUI())
    {
        filePath = "results/gui/TraCI_log.txt";
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        std::ostringstream fileName;
        fileName << std::setfill('0') << std::setw(3) << currentRun << "_TraCI_log.txt";
        filePath = "results/cmd/" + fileName.str();
    }

    FILE *filePtr = fopen (filePath.string().c_str(), "w");

    // write header
    fprintf (filePtr, "%-15s", "sentAt");
    fprintf (filePtr, "%-15s", "duration(ms)");
    fprintf (filePtr, "%-20s", "commandGroupId");
    fprintf (filePtr, "%-19s", "commandId");
    fprintf (filePtr, "%-40s \n\n", "commandName");

    // write body
    double oldTime = -1;
    for(auto &y : exchangedTraCIcommands)
    {
        // search for command name in TraCIcommandsMap
        auto it = TraCIcommandsMap.find(std::make_pair(y.commandGroupId, y.commandId));
        if(it == TraCIcommandsMap.end())
            error("can not find pair (0x%x, 0x%x) in TraCIcommandsMap", y.commandGroupId, y.commandId);

        // make the log more readable :)
        if(y.timeStamp != oldTime)
        {
            fprintf(filePtr, "\n");
            oldTime = y.timeStamp;
        }

        std::chrono::duration<double, std::milli> fp_ms = y.completeAt - y.sentAt;
        double duration = fp_ms.count();

        fprintf (filePtr, "%-15.2f", y.timeStamp);
        fprintf (filePtr, "%-15.2f", duration);
        fprintf (filePtr, "0x%-20x", y.commandGroupId);
        fprintf (filePtr, "0x%-15x", y.commandId);
        fprintf (filePtr, "%-40s \n", it->second.c_str());
    }

    fclose(filePtr);
}

}

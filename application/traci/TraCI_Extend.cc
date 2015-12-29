/****************************************************************************/
/// @file    TraCI_Extend.cc
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

#include "TraCI_Extend.h"
#include "TraCIConstants.h"

namespace VENTOS {

Define_Module(VENTOS::TraCI_Extend);

TraCI_Extend::~TraCI_Extend()
{

}


void TraCI_Extend::initialize(int stage)
{
    TraCI_Base::initialize(stage);

    if (stage == 1)
    {

    }
}


void TraCI_Extend::finish()
{
    TraCI_Base::finish();
}


void TraCI_Extend::handleSelfMsg(cMessage *msg)
{
    TraCI_Base::handleSelfMsg(msg);
}


// ################################################################
//                    generic methods for getters
// ################################################################

double TraCI_Extend::genericGetDouble(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
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


int32_t TraCI_Extend::genericGetInt(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
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


std::string TraCI_Extend::genericGetString(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
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


std::list<std::string> TraCI_Extend::genericGetStringList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
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


Coord TraCI_Extend::genericGetCoord(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
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

    return connection->traci2omnet(TraCICoord(x, y));
}


std::list<Coord> TraCI_Extend::genericGetCoordList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
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
        res.push_back(connection->traci2omnet(TraCICoord(x, y)));
    }

    ASSERT(buf.eof());

    return res;
}


uint8_t TraCI_Extend::genericGetUnsignedByte(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
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
Coord TraCI_Extend::genericGetCoordv2(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
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
std::vector<double> TraCI_Extend::genericGetBoundingBox(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
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


uint8_t* TraCI_Extend::genericGetArrayUnsignedInt(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
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
//                            simulation
// ################################################################

std::pair<uint32_t, std::string> TraCI_Extend::getVersion()
{
    bool success = false;
    TraCIBuffer buf = connection->queryOptional(CMD_GETVERSION, TraCIBuffer(), success);

    if (!success)
    {
        ASSERT(buf.eof());
        return std::pair<uint32_t, std::string>(0, "(unknown)");
    }

    uint8_t cmdLength; buf >> cmdLength;
    uint8_t commandResp; buf >> commandResp;
    ASSERT(commandResp == CMD_GETVERSION);
    uint32_t apiVersion; buf >> apiVersion;
    std::string serverVersion; buf >> serverVersion;
    ASSERT(buf.eof());

    return std::pair<uint32_t, std::string>(apiVersion, serverVersion);
}


// ####################
// CMD_GET_SIM_VARIABLE
// ####################

uint32_t TraCI_Extend::simulationGetLoadedVehiclesCount()
{
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

    return val;
}


std::list<std::string> TraCI_Extend::simulationGetLoadedVehiclesIDList()
{
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

    return res;

}


double* TraCI_Extend::simulationGetNetBoundary()
{
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

    buf >> boundaries[0];
    buf >> boundaries[1];
    buf >> boundaries[2];
    buf >> boundaries[3];

    ASSERT(buf.eof());

    return boundaries;
}


uint32_t TraCI_Extend::simulationGetMinExpectedNumber()
{
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

    return val;
}


uint32_t TraCI_Extend::simulationGetArrivedNumber()
{
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

    return val;
}


// #########################
// Control-related commands
// #########################

void TraCI_Extend::simulationTerminate()
{
    TraCIBuffer buf = connection->query(CMD_CLOSE, TraCIBuffer() << 0);

    uint32_t count;
    buf >> count;
}


// ################################################################
//                            vehicle
// ################################################################

// #########################
// CMD_GET_VEHICLE_VARIABLE
// #########################

// gets a list of all vehicles in the network (alphabetically!!!)
std::list<std::string> TraCI_Extend::vehicleGetIDList()
{
    return genericGetStringList(CMD_GET_VEHICLE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLE_VARIABLE);
}


uint32_t TraCI_Extend::vehicleGetIDCount()
{
    return genericGetInt(CMD_GET_VEHICLE_VARIABLE, "", ID_COUNT, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::vehicleGetSpeed(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_SPEED, RESPONSE_GET_VEHICLE_VARIABLE);
}

// value = 1 * stopped + 2  * parking + 4 * triggered + 8 * containerTriggered +
//        16 * busstop + 32 * containerstop

// isStopped           value & 1 == 1     whether the vehicle is stopped
// isStoppedParking    value & 2 == 2     whether the vehicle is parking (implies stopped)
// isStoppedTriggered  value & 12 > 0     whether the vehicle is stopped and waiting for a person or container
// isAtBusStop         value & 16 == 16   whether the vehicle is stopped at a bus stop
// isAtContainerStop   value & 32 == 32   whether the vehicle is stopped at a container stop
uint8_t TraCI_Extend::vehicleGetStopState(std::string nodeId)
{
    return genericGetUnsignedByte(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_STOPSTATE, RESPONSE_GET_VEHICLE_VARIABLE);
}


Coord TraCI_Extend::vehicleGetPosition(std::string nodeId)
{
    return genericGetCoordv2(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_POSITION, RESPONSE_GET_VEHICLE_VARIABLE);
}


std::string TraCI_Extend::vehicleGetEdgeID(std::string nodeId)
{
    return genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROAD_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}


std::string TraCI_Extend::vehicleGetLaneID(std::string nodeId)
{
    return genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}


uint32_t TraCI_Extend::vehicleGetLaneIndex(std::string nodeId)
{
    return genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_INDEX, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::vehicleGetLanePosition(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANEPOSITION, RESPONSE_GET_VEHICLE_VARIABLE);
}


std::string TraCI_Extend::vehicleGetTypeID(std::string nodeId)
{
    return genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TYPE, RESPONSE_GET_VEHICLE_VARIABLE);
}


std::string TraCI_Extend::vehicleGetRouteID(std::string nodeId)
{
    return genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROAD_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}


std::list<std::string> TraCI_Extend::vehicleGetRoute(std::string nodeId)
{
    return genericGetStringList(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_EDGES, RESPONSE_GET_VEHICLE_VARIABLE);
}


uint32_t TraCI_Extend::vehicleGetRouteIndex(std::string nodeId)
{
    return genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x69, RESPONSE_GET_VEHICLE_VARIABLE);
}


std::map<int,bestLanesEntry> TraCI_Extend::vehicleGetBestLanes(std::string nodeId)
{
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

    return final;
}


uint8_t* TraCI_Extend::vehicleGetColor(std::string nodeId)
{
    return genericGetArrayUnsignedInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_COLOR, RESPONSE_GET_VEHICLE_VARIABLE);
}


uint32_t TraCI_Extend::vehicleGetSignalStatus(std::string nodeId)
{
    return genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x5b, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::vehicleGetLength(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LENGTH, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::vehicleGetMinGap(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_MINGAP, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::vehicleGetMaxAccel(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ACCEL, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::vehicleGetMaxDecel(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_DECEL, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::vehicleGetTimeGap(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TAU, RESPONSE_GET_VEHICLE_VARIABLE);
}


std::string TraCI_Extend::vehicleGetClass(std::string nodeId)
{
    return genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_VEHICLECLASS, RESPONSE_GET_VEHICLE_VARIABLE);
}


std::vector<std::string> TraCI_Extend::vehicleGetLeader(std::string nodeId, double look_ahead_distance)
{
    uint8_t requestTypeId = TYPE_DOUBLE;
    uint8_t resultTypeId = TYPE_COMPOUND;
    uint8_t variableId = 0x68;
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

    return res;
}


double TraCI_Extend::vehicleGetCurrentAccel(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x70, RESPONSE_GET_VEHICLE_VARIABLE);
}


int TraCI_Extend::vehicleGetCarFollowingMode(std::string nodeId)
{
    return genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x71, RESPONSE_GET_VEHICLE_VARIABLE);
}


std::string TraCI_Extend::vehicleGetTLID(std::string nodeId)
{
    return genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x72, RESPONSE_GET_VEHICLE_VARIABLE);
}


char TraCI_Extend::vehicleGetTLLinkStatus(std::string nodeId)
{
    int result = genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x73, RESPONSE_GET_VEHICLE_VARIABLE);

    // covert to character
    return (char)result;
}


// #########################
// CMD_SET_VEHICLE_VARIABLE
// #########################

// Let the vehicle stop at the given edge, at the given position and lane.
// The vehicle will stop for the given duration.
void TraCI_Extend::vehicleSetStop(std::string nodeId, std::string edgeId, double stopPos, uint8_t laneId, double waitT, uint8_t flag)
{
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
}


void TraCI_Extend::vehicleResume(std::string nodeId)
{
    uint8_t variableId = CMD_RESUME;
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 0;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId
            << variableType << count);

    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetSpeed(std::string nodeId, double speed)
{
    uint8_t variableId = VAR_SPEED;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);

    ASSERT(buf.eof());
}


int32_t TraCI_Extend::vehicleBuildLaneChangeMode(uint8_t TraciLaneChangePriority, uint8_t RightDriveLC, uint8_t SpeedGainLC, uint8_t CooperativeLC, uint8_t StrategicLC)
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


void TraCI_Extend::vehicleSetLaneChangeMode(std::string nodeId, int32_t bitset)
{
    uint8_t variableId = 0xb6;
    uint8_t variableType = TYPE_INTEGER;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << bitset);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleChangeLane(std::string nodeId, uint8_t laneId, double duration)
{
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
}


void TraCI_Extend::vehicleSetRoute(std::string id, std::list<std::string> value)
{
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
}


void TraCI_Extend::vehicleSetRouteID(std::string nodeId, std::string routeID)
{
    uint8_t variableId = VAR_ROUTE_ID;
    uint8_t variableType = TYPE_STRING;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << routeID);

    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetColor(std::string nodeId, const TraCIColor& color)
{
    TraCIBuffer p;
    p << static_cast<uint8_t>(VAR_COLOR);
    p << nodeId;
    p << static_cast<uint8_t>(TYPE_COLOR) << color.red << color.green << color.blue << color.alpha;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, p);

    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetClass(std::string nodeId, std::string vClass)
{
    uint8_t variableId = VAR_VEHICLECLASS;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << vClass);

    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetMaxAccel(std::string nodeId, double value)
{
    uint8_t variableId = VAR_ACCEL;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetMaxDecel(std::string nodeId, double value)
{
    uint8_t variableId = VAR_DECEL;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());
}


// this changes vehicle type!
void TraCI_Extend::vehicleSetTimeGap(std::string nodeId, double value)
{
    uint8_t variableId = VAR_TAU;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleAdd(std::string vehicleId, std::string vehicleTypeId, std::string routeId, int32_t depart, double pos, double speed, uint8_t lane)
{
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
}


// todo:
//void TraCI_Extend::vehicleAddSimple(std::string vehicleId, std::string vehicleTypeId, std::string routeId, simtime_t emitTime_st, double emitPosition, double emitSpeed, int8_t emitLane)
//{
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
//}


void TraCI_Extend::vehicleRemove(std::string nodeId, uint8_t reason)
{
    uint8_t variableId = REMOVE;
    uint8_t variableType = TYPE_BYTE;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << reason);

    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetControllerParameters(std::string nodeId, std::string value)
{
    uint8_t variableId = 0x15;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetErrorGap(std::string nodeId, double value)
{
    uint8_t variableId = 0x20;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetErrorRelSpeed(std::string nodeId, double value)
{
    uint8_t variableId = 0x21;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetDowngradeToACC(std::string nodeId, bool value)
{
    uint8_t variableId = 0x17;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << (int)value);

    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetDebug(std::string nodeId, bool value)
{
    uint8_t variableId = 0x16;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << (int)value);

    ASSERT(buf.eof());
}



// ################################################################
//                          vehicle type
// ################################################################

// ############################
// CMD_GET_VEHICLETYPE_VARIABLE
// ############################

std::list<std::string> TraCI_Extend::vehicleTypeGetIDList()
{
    return genericGetStringList(CMD_GET_VEHICLETYPE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


uint32_t TraCI_Extend::vehicleTypeGetIDCount()
{
    return genericGetInt(CMD_GET_VEHICLETYPE_VARIABLE, "", ID_COUNT, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


double TraCI_Extend::vehicleTypeGetLength(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, VAR_LENGTH, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


double TraCI_Extend::vehicleTypeGetMaxSpeed(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, VAR_MAXSPEED, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


int TraCI_Extend::vehicleTypeGetControllerType(std::string nodeId)
{
    return genericGetInt(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, 0x02, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


int TraCI_Extend::vehicleTypeGetControllerNumber(std::string nodeId)
{
    return genericGetInt(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, 0x03, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


// ############################
// CMD_SET_VEHICLETYPE_VARIABLE
// ############################

void TraCI_Extend::vehicleTypeSetMaxSpeed(std::string nodeId, double speed)
{
    uint8_t variableId = VAR_MAXSPEED;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);

    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleTypeSetVint(std::string nodeId, double value)
{
    uint8_t variableId = 0x22;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);

    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleTypeSetComfAccel(std::string nodeId, double speed)
{
    uint8_t variableId = 0x23;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);

    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleTypeSetComfDecel(std::string nodeId, double speed)
{
    uint8_t variableId = 0x24;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);

    ASSERT(buf.eof());
}


// ################################################################
//                              route
// ################################################################

// ######################
// CMD_GET_ROUTE_VARIABLE
// ######################

std::list<std::string> TraCI_Extend::routeGetIDList()
{
    return genericGetStringList(CMD_GET_ROUTE_VARIABLE, "", ID_LIST, RESPONSE_GET_ROUTE_VARIABLE);
}


uint32_t TraCI_Extend::routeGetIDCount()
{
    return genericGetInt(CMD_GET_ROUTE_VARIABLE, "", ID_COUNT, RESPONSE_GET_ROUTE_VARIABLE);
}


std::list<std::string> TraCI_Extend::routeGetEdges(std::string routeID)
{
    return genericGetStringList(CMD_GET_ROUTE_VARIABLE, routeID, VAR_EDGES, RESPONSE_GET_ROUTE_VARIABLE);
}


// #######################
// CMD_SET_ROUTE_VARIABLE
// #######################

void TraCI_Extend::routeAdd(std::string name, std::list<std::string> route)
{
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

}


// ################################################################
//                              edge
// ################################################################

// ######################
// CMD_GET_EDGE_VARIABLE
// ######################

std::list<std::string> TraCI_Extend::edgeGetIDList()
{
    return genericGetStringList(CMD_GET_EDGE_VARIABLE, "", ID_LIST, RESPONSE_GET_EDGE_VARIABLE);
}


uint32_t TraCI_Extend::edgeGetIDCount()
{
    return genericGetInt(CMD_GET_EDGE_VARIABLE, "", ID_COUNT, RESPONSE_GET_EDGE_VARIABLE);
}


double TraCI_Extend::edgeGetMeanTravelTime(std::string Id)
{
    return genericGetDouble(CMD_GET_EDGE_VARIABLE, Id, VAR_CURRENT_TRAVELTIME, RESPONSE_GET_EDGE_VARIABLE);
}


uint32_t TraCI_Extend::edgeGetLastStepVehicleNumber(std::string Id)
{
    return genericGetInt(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_VEHICLE_NUMBER, RESPONSE_GET_EDGE_VARIABLE);
}


std::list<std::string> TraCI_Extend::edgeGetLastStepVehicleIDs(std::string Id)
{
    return genericGetStringList(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_EDGE_VARIABLE);
}


double TraCI_Extend::edgeGetLastStepMeanVehicleSpeed(std::string Id)
{
    return genericGetDouble(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_MEAN_SPEED, RESPONSE_GET_EDGE_VARIABLE);
}


double TraCI_Extend::edgeGetLastStepMeanVehicleLength(std::string Id)
{
    return genericGetDouble(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_LENGTH, RESPONSE_GET_EDGE_VARIABLE);
}


std::list<std::string> TraCI_Extend::edgeGetLastStepPersonIDs(std::string Id)
{
    return genericGetStringList(CMD_GET_EDGE_VARIABLE, Id, 0x1a, RESPONSE_GET_EDGE_VARIABLE);
}


// ######################
// CMD_SET_EDGE_VARIABLE
// ######################

void TraCI_Extend::edgeSetGlobalTravelTime(std::string edgeId, int32_t beginT, int32_t endT, double value)
{
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
}


// ################################################################
//                              lane
// ################################################################

// #####################
// CMD_GET_LANE_VARIABLE
// #####################

// gets a list of all lanes in the network
std::list<std::string> TraCI_Extend::laneGetIDList()
{
    return genericGetStringList(CMD_GET_LANE_VARIABLE, "", ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}


uint32_t TraCI_Extend::laneGetIDCount()
{
    return genericGetInt(CMD_GET_LANE_VARIABLE, "", ID_COUNT, RESPONSE_GET_LANE_VARIABLE);
}


uint8_t TraCI_Extend::laneGetLinkNumber(std::string laneId)
{
    return genericGetUnsignedByte(CMD_GET_LANE_VARIABLE, laneId, LANE_LINK_NUMBER, RESPONSE_GET_LANE_VARIABLE);
}


std::map<int,linkEntry> TraCI_Extend::laneGetLinks(std::string laneId)
{
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

    return final;
}


std::list<std::string> TraCI_Extend::laneGetAllowedClasses(std::string laneId)
{
    return genericGetStringList(CMD_GET_LANE_VARIABLE, laneId, LANE_ALLOWED, RESPONSE_GET_LANE_VARIABLE);
}


std::string TraCI_Extend::laneGetEdgeID(std::string laneId)
{
    return genericGetString(CMD_GET_LANE_VARIABLE, laneId, LANE_EDGE_ID, RESPONSE_GET_LANE_VARIABLE);
}


double TraCI_Extend::laneGetLength(std::string laneId)
{
    return genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_LENGTH, RESPONSE_GET_LANE_VARIABLE);
}


double TraCI_Extend::laneGetMaxSpeed(std::string laneId)
{
    return genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_MAXSPEED, RESPONSE_GET_LANE_VARIABLE);
}


uint32_t TraCI_Extend::laneGetLastStepVehicleNumber(std::string laneId)
{
    return genericGetInt(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_VEHICLE_NUMBER, RESPONSE_GET_LANE_VARIABLE);
}


std::list<std::string> TraCI_Extend::laneGetLastStepVehicleIDs(std::string laneId)
{
    return genericGetStringList(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}


double TraCI_Extend::laneGetLastStepMeanVehicleSpeed(std::string laneId)
{
    return genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_MEAN_SPEED, RESPONSE_GET_LANE_VARIABLE);
}


double TraCI_Extend::laneGetLastStepMeanVehicleLength(std::string laneId)
{
    return genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_LENGTH, RESPONSE_GET_LANE_VARIABLE);
}


// ######################
// CMD_SET_LANE_VARIABLE
// ######################

void TraCI_Extend::laneSetMaxSpeed(std::string laneId, double value)
{
    uint8_t variableId = VAR_MAXSPEED;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_LANE_VARIABLE, TraCIBuffer() << variableId << laneId << variableType << value);
    ASSERT(buf.eof());
}



// ################################################################
//                 loop detector (E1-Detectors)
// ################################################################

// ###############################
// CMD_GET_INDUCTIONLOOP_VARIABLE
// ###############################

std::list<std::string> TraCI_Extend::LDGetIDList()
{
    return genericGetStringList(CMD_GET_INDUCTIONLOOP_VARIABLE, "", 0x00, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


uint32_t TraCI_Extend::LDGetIDCount(std::string loopId)
{
    return genericGetInt(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x01, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


std::string TraCI_Extend::LDGetLaneID(std::string loopId)
{
    return genericGetString(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x51, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


double TraCI_Extend::LDGetPosition(std::string loopId)
{
    return genericGetDouble(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x42, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


uint32_t TraCI_Extend::LDGetLastStepVehicleNumber(std::string loopId)
{
    return genericGetInt(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x10, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


std::list<std::string> TraCI_Extend::LDGetLastStepVehicleIDs(std::string loopId)
{
    return genericGetStringList(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x12, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


double TraCI_Extend::LDGetLastStepMeanVehicleSpeed(std::string loopId)
{
    return genericGetDouble(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x11, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


double TraCI_Extend::LDGetElapsedTimeLastDetection(std::string loopId)
{
    return genericGetDouble(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x16, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


std::vector<std::string> TraCI_Extend::LDGetLastStepVehicleData(std::string loopId)
{
    uint8_t resultTypeId = TYPE_COMPOUND;   // note: type is compound!
    uint8_t variableId = 0x17;
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

    return res;
}


// ################################################################
//                lane area detector (E2-Detectors)
// ################################################################

// ###############################
// CMD_GET_AREAL_DETECTOR_VARIABLE
// ###############################

std::list<std::string> TraCI_Extend::LADGetIDList()
{
    return genericGetStringList(0xad, "", 0x00, 0xbd);
}


uint32_t TraCI_Extend::LADGetIDCount(std::string loopId)
{
    return genericGetInt(0xad, loopId, 0x01, 0xbd);
}

std::string TraCI_Extend::LADGetLaneID(std::string loopId)
{
    return genericGetString(0xad, loopId, 0x51, 0xbd);
}


uint32_t TraCI_Extend::LADGetLastStepVehicleNumber(std::string loopId)
{
    return genericGetInt(0xad, loopId, 0x10, 0xbd);
}


std::list<std::string> TraCI_Extend::LADGetLastStepVehicleIDs(std::string loopId)
{
    return genericGetStringList(0xad, loopId, 0x12, 0xbd);
}


double TraCI_Extend::LADGetLastStepMeanVehicleSpeed(std::string loopId)
{
    return genericGetDouble(0xad, loopId, 0x11, 0xbd);
}


uint32_t TraCI_Extend::LADGetLastStepVehicleHaltingNumber(std::string loopId)
{
    return genericGetInt(0xad, loopId, 0x14, 0xbd);
}



// ################################################################
//                          traffic light
// ################################################################

// ###################
// CMD_GET_TL_VARIABLE
// ###################

std::list<std::string> TraCI_Extend::TLGetIDList()
{
    return genericGetStringList(CMD_GET_TL_VARIABLE, "",ID_LIST, RESPONSE_GET_TL_VARIABLE);
}


uint32_t TraCI_Extend::TLGetIDCount()
{
    return genericGetInt(CMD_GET_TL_VARIABLE, "",ID_COUNT, RESPONSE_GET_TL_VARIABLE);
}


std::list<std::string> TraCI_Extend::TLGetControlledLanes(std::string TLid)
{
    return genericGetStringList(CMD_GET_TL_VARIABLE, TLid, TL_CONTROLLED_LANES, RESPONSE_GET_TL_VARIABLE);
}


std::map<int,std::vector<std::string>> TraCI_Extend::TLGetControlledLinks(std::string TLid)
{
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

    return myMap;
}


std::string TraCI_Extend::TLGetProgram(std::string TLid)
{
    return genericGetString(CMD_GET_TL_VARIABLE, TLid, TL_CURRENT_PROGRAM, RESPONSE_GET_TL_VARIABLE);
}


uint32_t TraCI_Extend::TLGetPhase(std::string TLid)
{
    return genericGetInt(CMD_GET_TL_VARIABLE, TLid, TL_CURRENT_PHASE, RESPONSE_GET_TL_VARIABLE);
}


std::string TraCI_Extend::TLGetState(std::string TLid)
{
    return genericGetString(CMD_GET_TL_VARIABLE, TLid, TL_RED_YELLOW_GREEN_STATE, RESPONSE_GET_TL_VARIABLE);
}


uint32_t TraCI_Extend::TLGetPhaseDuration(std::string TLid)
{
    return genericGetInt(CMD_GET_TL_VARIABLE, TLid, TL_PHASE_DURATION, RESPONSE_GET_TL_VARIABLE);
}


uint32_t TraCI_Extend::TLGetNextSwitchTime(std::string TLid)
{
    return genericGetInt(CMD_GET_TL_VARIABLE, TLid, TL_NEXT_SWITCH, RESPONSE_GET_TL_VARIABLE);
}


// ###################
// CMD_SET_TL_VARIABLE
// ###################

void TraCI_Extend::TLSetProgram(std::string TLid, std::string value)
{
    uint8_t variableId = TL_PROGRAM;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::TLSetPhaseIndex(std::string TLid, int value)
{
    uint8_t variableId = TL_PHASE_INDEX;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::TLSetPhaseDuration(std::string TLid, int value)
{
    uint8_t variableId = TL_PHASE_DURATION;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::TLSetState(std::string TLid, std::string value)
{
    uint8_t variableId = TL_RED_YELLOW_GREEN_STATE;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());
}


// ################################################################
//                             Junction
// ################################################################

// #########################
// CMD_GET_JUNCTION_VARIABLE
// #########################

std::list<std::string> TraCI_Extend::junctionGetIDList()
{
    return genericGetStringList(CMD_GET_JUNCTION_VARIABLE, "",ID_LIST, RESPONSE_GET_JUNCTION_VARIABLE);
}


uint32_t TraCI_Extend::junctionGetIDCount()
{
    return genericGetInt(CMD_GET_JUNCTION_VARIABLE, "",ID_COUNT, RESPONSE_GET_JUNCTION_VARIABLE);
}


Coord TraCI_Extend::junctionGetPosition(std::string id)
{
    return genericGetCoordv2(CMD_GET_JUNCTION_VARIABLE, id, VAR_POSITION, RESPONSE_GET_JUNCTION_VARIABLE);
}


// ################################################################
//                               GUI
// ################################################################

// #####################
// CMD_GET_GUI_VARIABLE
// #####################

Coord TraCI_Extend::GUIGetOffset()
{
    return genericGetCoordv2(CMD_GET_GUI_VARIABLE, "View #0", VAR_VIEW_OFFSET, RESPONSE_GET_GUI_VARIABLE);
}


std::vector<double> TraCI_Extend::GUIGetBoundry()
{
    return genericGetBoundingBox(CMD_GET_GUI_VARIABLE, "View #0", VAR_VIEW_BOUNDARY, RESPONSE_GET_GUI_VARIABLE);
}


// #####################
// CMD_SET_GUI_VARIABLE
// #####################

void TraCI_Extend::GUISetZoom(double value)
{
    uint8_t variableId = VAR_VIEW_ZOOM;
    std::string viewID = "View #0";
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::GUISetOffset(double x, double y)
{
    uint8_t variableId = VAR_VIEW_OFFSET;
    std::string viewID = "View #0";
    uint8_t variableType = POSITION_2D;

    TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << x << y);
    ASSERT(buf.eof());
}


// very slow!
void TraCI_Extend::GUISetTrackVehicle(std::string nodeId)
{
    uint8_t variableId = VAR_TRACK_VEHICLE;
    std::string viewID = "View #0";
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << nodeId);
    ASSERT(buf.eof());
}


// ################################################################
//                               polygon
// ################################################################

// ########################
// CMD_GET_POLYGON_VARIABLE
// ########################

std::list<std::string> TraCI_Extend::polygonGetIDList()
{
    return genericGetStringList(CMD_GET_POLYGON_VARIABLE, "", ID_LIST, RESPONSE_GET_POLYGON_VARIABLE);
}


uint32_t TraCI_Extend::polygonGetIDCount()
{
    return genericGetInt(CMD_GET_POLYGON_VARIABLE, "",ID_COUNT, RESPONSE_GET_POLYGON_VARIABLE);
}


std::list<Coord> TraCI_Extend::polygonGetShape(std::string polyId)
{
    return genericGetCoordList(CMD_GET_POLYGON_VARIABLE, polyId, VAR_SHAPE, RESPONSE_GET_POLYGON_VARIABLE);
}


std::string TraCI_Extend::polygonGetTypeID(std::string polyId)
{
    return genericGetString(CMD_GET_POLYGON_VARIABLE, polyId, VAR_TYPE, RESPONSE_GET_POLYGON_VARIABLE);
}


// ########################
// CMD_SET_POLYGON_VARIABLE
// ########################

void TraCI_Extend::polygonAddTraCI(std::string polyId, std::string polyType, const TraCIColor& color, bool filled, int32_t layer, const std::list<TraCICoord>& points)
{
    TraCIBuffer p;

    p << static_cast<uint8_t>(ADD) << polyId;
    p << static_cast<uint8_t>(TYPE_COMPOUND) << static_cast<int32_t>(5);
    p << static_cast<uint8_t>(TYPE_STRING) << polyType;
    p << static_cast<uint8_t>(TYPE_COLOR) << color.red << color.green << color.blue << color.alpha;
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
}


void TraCI_Extend::polygonAdd(std::string polyId, std::string polyType, const TraCIColor& color, bool filled, int32_t layer, const std::list<Coord>& points)
{
    TraCIBuffer p;

    p << static_cast<uint8_t>(ADD) << polyId;
    p << static_cast<uint8_t>(TYPE_COMPOUND) << static_cast<int32_t>(5);
    p << static_cast<uint8_t>(TYPE_STRING) << polyType;
    p << static_cast<uint8_t>(TYPE_COLOR) << color.red << color.green << color.blue << color.alpha;
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
}


void TraCI_Extend::polygonSetFilled(std::string polyId, uint8_t filled)
{
    uint8_t variableId = VAR_FILL;
    uint8_t variableType = TYPE_UBYTE;

    TraCIBuffer buf = connection->query(CMD_SET_POLYGON_VARIABLE, TraCIBuffer() << variableId << polyId << variableType << filled);
    ASSERT(buf.eof());
}


// ################################################################
//                               POI
// ################################################################

void TraCI_Extend::addPoi(std::string poiId, std::string poiType, const TraCIColor& color, int32_t layer, const Coord& pos_)
{
    TraCIBuffer p;

    TraCICoord pos = connection->omnet2traci(pos_);
    p << static_cast<uint8_t>(ADD) << poiId;
    p << static_cast<uint8_t>(TYPE_COMPOUND) << static_cast<int32_t>(4);
    p << static_cast<uint8_t>(TYPE_STRING) << poiType;
    p << static_cast<uint8_t>(TYPE_COLOR) << color.red << color.green << color.blue << color.alpha;
    p << static_cast<uint8_t>(TYPE_INTEGER) << layer;
    p << pos;

    TraCIBuffer buf = connection->query(CMD_SET_POI_VARIABLE, p);
    ASSERT(buf.eof());
}


// ################################################################
//                               person
// ################################################################

// ##############
// CMD_GET_PERSON
// ##############

std::list<std::string> TraCI_Extend::personGetIDList()
{
    return genericGetStringList(0xae, "", ID_LIST, 0xbe);
}


uint32_t TraCI_Extend::personGetIDCount()
{
    return genericGetInt(0xae, "", ID_COUNT, 0xbe);
}


std::string TraCI_Extend::personGetTypeID(std::string pId)
{
    return genericGetString(0xae, pId, VAR_TYPE, 0xbe);
}


Coord TraCI_Extend::personGetPosition(std::string pId)
{
    return genericGetCoordv2(0xae, pId, VAR_POSITION, 0xbe);
}


std::string TraCI_Extend::personGetEdgeID(std::string pId)
{
    return genericGetString(0xae, pId, VAR_ROAD_ID, 0xbe);
}


double TraCI_Extend::personGetEdgePosition(std::string pId)
{
    return genericGetDouble(0xae, pId, 0x56, 0xbe);
}


double TraCI_Extend::personGetSpeed(std::string pId)
{
    return genericGetDouble(0xae, pId, VAR_SPEED, 0xbe);
}


std::string TraCI_Extend::personGetNextEdge(std::string pId)
{
    return genericGetString(0xae, pId, 0xc1, 0xbe);
}

}

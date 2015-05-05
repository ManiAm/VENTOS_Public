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

#define MYDEBUG EV

namespace VENTOS {

Define_Module(VENTOS::TraCI_Extend);

TraCI_Extend::~TraCI_Extend()
{

}


void TraCI_Extend::initialize(int stage)
{
    TraCIScenarioManager::initialize(stage);

    if (stage == 1)
    {
        VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        SUMO_Path = simulation.getSystemModule()->par("SUMODirectory").stringValue();
        SUMO_FullPath = VENTOS_FullPath / SUMO_Path;

        // check if this directory is valid?
        if( !exists( SUMO_FullPath ) )
        {
            error("SUMO directory is not valid! Check it again.");
        }
    }
}


void TraCI_Extend::finish()
{
    TraCIScenarioManager::finish();
}


void TraCI_Extend::handleSelfMsg(cMessage *msg)
{
    TraCIScenarioManager::handleSelfMsg(msg);
}


void TraCI_Extend::init_traci()
{
    // get the version of Python launchd
    pair<uint32_t, string> versionP = getCommandInterface()->getVersion();
    uint32_t apiVersionP = versionP.first;
    string serverVersionP = versionP.second;
    EV << "TraCI launchd reports apiVersion: " << apiVersionP << " and serverVersion: " << serverVersionP << endl;
    ASSERT(apiVersionP == 1);

    // send the launch file to python script
    sendLaunchFile();

    // #################################################
    // Exact copy of TraCIScenarioManager::init_traci()
    // #################################################

    // get the version of SUMO TraCI API
    std::pair<uint32_t, std::string> versionS = getCommandInterface()->getVersion();
    uint32_t apiVersionS = versionS.first;
    std::string serverVersionS = versionS.second;

    if (apiVersionS == 10) {
        MYDEBUG << "TraCI server \"" << serverVersionS << "\" reports API version " << apiVersionS << endl;
    }
    else {
        error("TraCI server \"%s\" reports API version %d, which is unsupported. We recommend using SUMO 0.21.0.", serverVersionS.c_str(), apiVersionS);
    }

    {
    // query road network boundaries
    TraCIBuffer buf = connection->query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_NET_BOUNDING_BOX) << std::string("sim0"));
    uint8_t cmdLength_resp; buf >> cmdLength_resp;
    uint8_t commandId_resp; buf >> commandId_resp; ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
    uint8_t variableId_resp; buf >> variableId_resp; ASSERT(variableId_resp == VAR_NET_BOUNDING_BOX);
    std::string simId; buf >> simId;
    uint8_t typeId_resp; buf >> typeId_resp; ASSERT(typeId_resp == TYPE_BOUNDINGBOX);
    double x1; buf >> x1;
    double y1; buf >> y1;
    double x2; buf >> x2;
    double y2; buf >> y2;
    ASSERT(buf.eof());

    TraCICoord netbounds1 = TraCICoord(x1, y1);
    TraCICoord netbounds2 = TraCICoord(x2, y2);
    MYDEBUG << "TraCI reports network boundaries (" << x1 << ", " << y1 << ")-(" << x2 << ", " << y2 << ")" << endl;
    connection->setNetbounds(netbounds1, netbounds2, par("margin"));
    if ((connection->traci2omnet(netbounds2).x > world->getPgs()->x) || (connection->traci2omnet(netbounds1).y > world->getPgs()->y)) MYDEBUG << "WARNING: Playground size (" << world->getPgs()->x << ", " << world->getPgs()->y << ") might be too small for vehicle at network bounds (" << connection->traci2omnet(netbounds2).x << ", " << connection->traci2omnet(netbounds1).y << ")" << endl;
    }

    {
    // subscribe to list of departed and arrived vehicles, as well as simulation time
    uint32_t beginTime = 0;
    uint32_t endTime = 0x7FFFFFFF;
    std::string objectId = "";
    uint8_t variableNumber = 7;
    uint8_t variable1 = VAR_DEPARTED_VEHICLES_IDS;
    uint8_t variable2 = VAR_ARRIVED_VEHICLES_IDS;
    uint8_t variable3 = VAR_TIME_STEP;
    uint8_t variable4 = VAR_TELEPORT_STARTING_VEHICLES_IDS;
    uint8_t variable5 = VAR_TELEPORT_ENDING_VEHICLES_IDS;
    uint8_t variable6 = VAR_PARKING_STARTING_VEHICLES_IDS;
    uint8_t variable7 = VAR_PARKING_ENDING_VEHICLES_IDS;
    TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_SIM_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1 << variable2 << variable3 << variable4 << variable5 << variable6 << variable7);
    processSubcriptionResult(buf);
    ASSERT(buf.eof());
    }

    {
    // subscribe to list of vehicle ids
    uint32_t beginTime = 0;
    uint32_t endTime = 0x7FFFFFFF;
    std::string objectId = "";
    uint8_t variableNumber = 1;
    uint8_t variable1 = ID_LIST;
    TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_VEHICLE_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1);
    processSubcriptionResult(buf);
    ASSERT(buf.eof());
    }

    ObstacleControl* obstacles = ObstacleControlAccess().getIfExists();
    if (obstacles) {
        {
            // get list of polygons
            std::list<std::string> ids = getCommandInterface()->getPolygonIds();
            for (std::list<std::string>::iterator i = ids.begin(); i != ids.end(); ++i) {
                std::string id = *i;
                std::string typeId = getCommandInterface()->polygon(id).getTypeId();
                if (typeId == "building") {
                    std::list<Coord> coords = getCommandInterface()->polygon(id).getShape();
                    Obstacle obs(id, 9, .4); // each building gets attenuation of 9 dB per wall, 0.4 dB per meter
                    std::vector<Coord> shape;
                    std::copy(coords.begin(), coords.end(), std::back_inserter(shape));
                    obs.setShape(shape);
                    obstacles->add(obs);
                }
            }
        }
    }
} // method end


void TraCI_Extend::sendLaunchFile()
{
    string launchFile = par("launchFile").stringValue();
    boost::filesystem::path launchFullPath = SUMO_FullPath / launchFile;

    file<> xmlFile( launchFullPath.string().c_str() );
    xml_document<> doc;
    doc.parse<0>(xmlFile.data());

    // create a node called basedir
    xml_node<> *basedir = doc.allocate_node(node_element, "basedir");

    // append attribute to basedir
    xml_attribute<> *attr = doc.allocate_attribute("path", SUMO_FullPath.c_str());
    basedir->append_attribute(attr);

    // append basedir to the launch
    xml_node<> *node = doc.first_node("launch");
    node->append_node(basedir);

    // seed value to set in launch configuration, if missing (-1: current run number)
    int seed = par("seed");

    if (seed == -1)
    {
        // default seed is current repetition
        const char* seed_s = cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getVariable(CFGVAR_RUNNUMBER);
        seed = atoi(seed_s);
    }

    // create a node called seed
    xml_node<> *seedN = doc.allocate_node(node_element, "seed");

    // append attribute to seed
    stringstream ss; ss << seed;
    xml_attribute<> *attr1 = doc.allocate_attribute( "value", ss.str().c_str() );
    seedN->append_attribute(attr1);

    // append seed to the launch
    xml_node<> *node1 = doc.first_node("launch");
    node1->append_node(seedN);

    // convert to string
    std::stringstream os;
    os << doc;
    string contents = os.str();

    // send the string of the launch file to python
    uint8_t commandId = 0x75;
    TraCIBuffer buf;
    buf << string("sumo-launchd.launch.xml") << contents;
    getCommandInterface()->connection.sendMessage(makeTraCICommand(commandId, buf));

    TraCIBuffer obuf( getCommandInterface()->connection.receiveMessage() );
    uint8_t cmdLength; obuf >> cmdLength;
    uint8_t commandResp; obuf >> commandResp;
    if (commandResp != commandId)
    {
        error("Expected response to command %d, but got one for command %d", commandId, commandResp);
    }

    uint8_t result; obuf >> result;
    string description; obuf >> description;
    if (result != RTYPE_OK)
    {
        EV << "Warning: Received non-OK response from TraCI server to command " << commandId << ":" << description.c_str() << endl;
    }
}


// same as genericGetCoordv, but no conversion to omnet++ coordinates at the end
Coord TraCI_Extend::genericGetCoordv2(uint8_t commandId, string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = POSITION_2D;
    double x;
    double y;

    TraCIBuffer buf = getCommandInterface()->connection.query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    string objectId_r; buf >> objectId_r;
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
vector<double> TraCI_Extend::genericGetBoundingBox(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_BOUNDINGBOX;
    double LowerLeftX;
    double LowerLeftY;
    double UpperRightX;
    double UpperRightY;
    vector<double> res;

    TraCIBuffer buf = getCommandInterface()->connection.query(commandId, TraCIBuffer() << variableId << objectId);

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


uint8_t* TraCI_Extend::genericGetArrayUnsignedInt(uint8_t commandId, string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_COLOR;
    uint8_t* color = new uint8_t[4]; // RGBA

    TraCIBuffer buf = getCommandInterface()->connection.query(commandId, TraCIBuffer() << variableId << objectId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    string objectId_r; buf >> objectId_r;
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

// ####################
// CMD_GET_SIM_VARIABLE
// ####################

double* TraCI_Extend::simulationGetNetBoundary()
{
    // query road network boundaries
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_NET_BOUNDING_BOX) << string("sim0"));

    uint8_t cmdLength_resp; buf >> cmdLength_resp;
    uint8_t commandId_resp; buf >> commandId_resp; ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
    uint8_t variableId_resp; buf >> variableId_resp; ASSERT(variableId_resp == VAR_NET_BOUNDING_BOX);
    string simId; buf >> simId;
    uint8_t typeId_resp; buf >> typeId_resp; ASSERT(typeId_resp == TYPE_BOUNDINGBOX);

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
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_MIN_EXPECTED_VEHICLES) << string("sim0"));

    uint8_t cmdLength_resp; buf >> cmdLength_resp;
    uint8_t commandId_resp; buf >> commandId_resp; ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
    uint8_t variableId_resp; buf >> variableId_resp; ASSERT(variableId_resp == VAR_MIN_EXPECTED_VEHICLES);
    string simId; buf >> simId;
    uint8_t typeId_resp; buf >> typeId_resp; ASSERT(typeId_resp == TYPE_INTEGER);

    uint32_t val;
    buf >> val;

    return val;
}


uint32_t TraCI_Extend::simulationGetArrivedNumber()
{
    // query road network boundaries
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_ARRIVED_VEHICLES_NUMBER) << string("sim0"));

    uint8_t cmdLength_resp; buf >> cmdLength_resp;
    uint8_t commandId_resp; buf >> commandId_resp; ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
    uint8_t variableId_resp; buf >> variableId_resp; ASSERT(variableId_resp == VAR_ARRIVED_VEHICLES_NUMBER);
    string simId; buf >> simId;
    uint8_t typeId_resp; buf >> typeId_resp; ASSERT(typeId_resp == TYPE_INTEGER);

    uint32_t val;
    buf >> val;

    return val;
}


// #########################
// Control-related commands
// #########################

void TraCI_Extend::simulationTerminate()
{
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_CLOSE, TraCIBuffer() << 0);

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
list<string> TraCI_Extend::vehicleGetIDList()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_VEHICLE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLE_VARIABLE);
}


uint32_t TraCI_Extend::vehicleGetIDCount()
{
    return getCommandInterface()->genericGetInt(CMD_GET_VEHICLE_VARIABLE, "", ID_COUNT, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::vehicleGetSpeed(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_SPEED, RESPONSE_GET_VEHICLE_VARIABLE);
}

// value = 1 * stopped + 2  * parking + 4 * triggered + 8 * containerTriggered +
//        16 * busstop + 32 * containerstop

// isStopped           value & 1 == 1     whether the vehicle is stopped
// isStoppedParking    value & 2 == 2     whether the vehicle is parking (implies stopped)
// isStoppedTriggered  value & 12 > 0     whether the vehicle is stopped and waiting for a person or container
// isAtBusStop         value & 16 == 16   whether the vehicle is stopped at a bus stop
// isAtContainerStop   value & 32 == 32   whether the vehicle is stopped at a container stop
uint8_t* TraCI_Extend::vehicleGetStopState(string nodeId)
{
    return genericGetArrayUnsignedInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_STOPSTATE, RESPONSE_GET_VEHICLE_VARIABLE);
}


Coord TraCI_Extend::vehicleGetPosition(string nodeId)
{
    return genericGetCoordv2(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_POSITION, RESPONSE_GET_VEHICLE_VARIABLE);
}


string TraCI_Extend::vehicleGetEdgeID(std::string nodeId)
{
    return getCommandInterface()->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROAD_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}


std::string TraCI_Extend::vehicleGetLaneID(string nodeId)
{
    return getCommandInterface()->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}


uint32_t TraCI_Extend::vehicleGetLaneIndex(string nodeId)
{
    return getCommandInterface()->genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_INDEX, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::vehicleGetLanePosition(std::string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANEPOSITION, RESPONSE_GET_VEHICLE_VARIABLE);
}


string TraCI_Extend::vehicleGetTypeID(string nodeId)
{
    return getCommandInterface()->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TYPE, RESPONSE_GET_VEHICLE_VARIABLE);
}


string TraCI_Extend::vehicleGetRouteID(string nodeId)
{
    return getCommandInterface()->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROAD_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}


list<string> TraCI_Extend::vehicleGetRoute(string nodeId)
{
    return getCommandInterface()->genericGetStringList(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_EDGES, RESPONSE_GET_VEHICLE_VARIABLE);
}


string TraCI_Extend::vehicleGetBestLanes(string nodeId)
{
    return getCommandInterface()->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_BEST_LANES, RESPONSE_GET_VEHICLE_VARIABLE);
}


uint8_t* TraCI_Extend::vehicleGetColor(string nodeId)
{
    return genericGetArrayUnsignedInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_COLOR, RESPONSE_GET_VEHICLE_VARIABLE);
}


uint8_t* TraCI_Extend::vehicleGetSignals(string nodeId)
{
    return genericGetArrayUnsignedInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x5b, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::vehicleGetLength(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LENGTH, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::vehicleGetMinGap(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_MINGAP, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::vehicleGetMaxAccel(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ACCEL, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::vehicleGetMaxDecel(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_DECEL, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::vehicleGetTimeGap(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TAU, RESPONSE_GET_VEHICLE_VARIABLE);
}


string TraCI_Extend::vehicleGetClass(string nodeId)
{
    return getCommandInterface()->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_VEHICLECLASS, RESPONSE_GET_VEHICLE_VARIABLE);
}


vector<string> TraCI_Extend::vehicleGetLeader(string nodeId, double look_ahead_distance)
{
    uint8_t requestTypeId = TYPE_DOUBLE;
    uint8_t resultTypeId = TYPE_COMPOUND;
    uint8_t variableId = 0x68;
    uint8_t responseId = RESPONSE_GET_VEHICLE_VARIABLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_GET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << requestTypeId << look_ahead_distance);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == nodeId);

    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint32_t count; buf >> count;

    // now we start getting real data that we are looking for
    vector<string> res;

    uint8_t dType; buf >> dType;
    string id; buf >> id;
    res.push_back(id);

    uint8_t dType2; buf >> dType2;
    double len; buf >> len;

    // the distance does not include minGap
    // we will add minGap to the len
    len = len + vehicleGetMinGap(nodeId);

    // now convert len to string
    ostringstream s;
    s << len;
    res.push_back(s.str());

    ASSERT(buf.eof());

    return res;
}


double TraCI_Extend::vehicleGetCurrentAccel(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x70, RESPONSE_GET_VEHICLE_VARIABLE);
}


int TraCI_Extend::vehicleGetCarFollowingMode(string nodeId)
{
    return getCommandInterface()->genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x71, RESPONSE_GET_VEHICLE_VARIABLE);
}


int TraCI_Extend::vehicleGetTrafficLightAhead(string nodeId)
{
    return getCommandInterface()->genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x72, RESPONSE_GET_VEHICLE_VARIABLE);
}


// #########################
// CMD_SET_VEHICLE_VARIABLE
// #########################

// Let the vehicle stop at the given edge, at the given position and lane.
// The vehicle will stop for the given duration.
void TraCI_Extend::vehicleSetStop(string nodeId, string edgeId, double stopPos, uint8_t laneId, double waitT, uint8_t flag)
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

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId
                                                                                       << variableType << count
                                                                                       << edgeIdT << edgeId
                                                                                       << stopPosT << stopPos
                                                                                       << stopLaneT << laneId
                                                                                       << durationT << duration
                                                                                       << flagT << flag
                                                                                       );
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleResume(string nodeId)
{
    uint8_t variableId = CMD_RESUME;
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 0;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId
                                                                                       << variableType << count
                                                                                       );
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetSpeed(string nodeId, double speed)
{
    uint8_t variableId = VAR_SPEED;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);
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


void TraCI_Extend::vehicleSetLaneChangeMode(string nodeId, int32_t bitset)
{
    uint8_t variableId = 0xb6;
    uint8_t variableType = TYPE_INTEGER;
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << bitset);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleChangeLane(string nodeId, uint8_t laneId, double duration)
{
    uint8_t variableId = CMD_CHANGELANE;
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 2;
    uint8_t laneIdT = TYPE_BYTE;
    uint8_t durationT = TYPE_INTEGER;
    uint32_t durationMS = duration * 1000;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId
                                                                                       << variableType << count
                                                                                       << laneIdT << laneId
                                                                                       << durationT << durationMS
                                                                                       );
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetRoute(string id, list<string> value)
{
    uint8_t variableId = VAR_ROUTE;
    uint8_t variableTypeSList = TYPE_STRINGLIST;

    TraCIBuffer buffer;
    buffer << variableId << id << variableTypeSList << (int32_t)value.size();
    for(list<string>::iterator str = value.begin(); str != value.end(); str++)
    {
        buffer << (int32_t)str->length();
        for(unsigned int i = 0; i < str->length(); i++)
            buffer << (int8_t)(*str)[i];
    }
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, buffer);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetRouteID(string nodeId, string routeID)
{
    uint8_t variableId = VAR_ROUTE_ID;
    uint8_t variableType = TYPE_STRING;
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << routeID);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetColor(std::string nodeId, const TraCIColor& color)
{
    TraCIBuffer p;
    p << static_cast<uint8_t>(VAR_COLOR);
    p << nodeId;
    p << static_cast<uint8_t>(TYPE_COLOR) << color.red << color.green << color.blue << color.alpha;
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, p);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetClass(string nodeId, string vClass)
{
    uint8_t variableId = VAR_VEHICLECLASS;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << vClass);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetMaxAccel(string nodeId, double value)
{
    uint8_t variableId = VAR_ACCEL;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetMaxDecel(string nodeId, double value)
{
    uint8_t variableId = VAR_DECEL;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


// this changes vehicle type!
void TraCI_Extend::vehicleSetTimeGap(string nodeId, double value)
{
    uint8_t variableId = VAR_TAU;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleAdd(string vehicleId, string vehicleTypeId, string routeId, int32_t depart, double pos, double speed, uint8_t lane)
{
    uint8_t variableId = ADD;
    uint8_t variableType = TYPE_COMPOUND;
    uint8_t variableTypeS = TYPE_STRING;
    uint8_t variableTypeI = TYPE_INTEGER;
    uint8_t variableTypeD = TYPE_DOUBLE;
    uint8_t variableTypeB = TYPE_BYTE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << vehicleId
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
                                                                         << lane          // departure lane
                                                                         );
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleRemove(string nodeId, uint8_t reason)
{
    uint8_t variableId = REMOVE;
    uint8_t variableType = TYPE_BYTE;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << reason);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetControllerParameters(string nodeId, string value)
{
    uint8_t variableId = 0x15;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetErrorGap(string nodeId, double value)
{
    uint8_t variableId = 0x20;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetErrorRelSpeed(string nodeId, double value)
{
    uint8_t variableId = 0x21;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetDowngradeToACC(string nodeId, bool value)
{
    uint8_t variableId = 0x17;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << (int)value);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleSetDebug(string nodeId, bool value)
{
    uint8_t variableId = 0x16;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << (int)value);
    ASSERT(buf.eof());
}



// ################################################################
//                          vehicle type
// ################################################################

// ############################
// CMD_GET_VEHICLETYPE_VARIABLE
// ############################

list<string> TraCI_Extend::vehicleTypeGetIDList()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_VEHICLETYPE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


uint32_t TraCI_Extend::vehicleTypeGetIDCount()
{
    return getCommandInterface()->genericGetInt(CMD_GET_VEHICLETYPE_VARIABLE, "", ID_COUNT, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


double TraCI_Extend::vehicleTypeGetLength(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, VAR_LENGTH, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


double TraCI_Extend::vehicleTypeGetMaxSpeed(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, VAR_MAXSPEED, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


int TraCI_Extend::vehicleTypeGetControllerType(string nodeId)
{
    return getCommandInterface()->genericGetInt(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, 0x02, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


int TraCI_Extend::vehicleTypeGetControllerNumber(string nodeId)
{
    return getCommandInterface()->genericGetInt(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, 0x03, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


// ############################
// CMD_SET_VEHICLETYPE_VARIABLE
// ############################

void TraCI_Extend::vehicleTypeSetMaxSpeed(string nodeId, double speed)
{
    uint8_t variableId = VAR_MAXSPEED;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleTypeSetVint(string nodeId, double value)
{
    uint8_t variableId = 0x22;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleTypeSetComfAccel(string nodeId, double speed)
{
    uint8_t variableId = 0x23;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);
    ASSERT(buf.eof());
}


void TraCI_Extend::vehicleTypeSetComfDecel(string nodeId, double speed)
{
    uint8_t variableId = 0x24;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);
    ASSERT(buf.eof());
}


// ################################################################
//                              route
// ################################################################

// ######################
// CMD_GET_ROUTE_VARIABLE
// ######################

list<string> TraCI_Extend::routeGetIDList()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_ROUTE_VARIABLE, "", ID_LIST, RESPONSE_GET_ROUTE_VARIABLE);
}


uint32_t TraCI_Extend::routeGetIDCount()
{
    return getCommandInterface()->genericGetInt(CMD_GET_ROUTE_VARIABLE, "", ID_COUNT, RESPONSE_GET_ROUTE_VARIABLE);
}


list<string> TraCI_Extend::routeGetEdges(string routeID)
{
    return getCommandInterface()->genericGetStringList(CMD_GET_ROUTE_VARIABLE, routeID, VAR_EDGES, RESPONSE_GET_ROUTE_VARIABLE);
}


// #######################
// CMD_SET_ROUTE_VARIABLE
// #######################

void TraCI_Extend::routeAdd(string name, list<string> route)
{
    uint8_t variableId = ADD;
    uint8_t variableTypeS = TYPE_STRINGLIST;

    TraCIBuffer buffer;
    buffer << variableId << name << variableTypeS << (int32_t)route.size();
    for(list<string>::iterator str = route.begin(); str != route.end(); str++)
    {
        buffer << (int32_t)str->length();
        for(unsigned int i = 0; i < str->length(); i++)
            buffer << (int8_t)(*str)[i];
    }

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_ROUTE_VARIABLE, buffer);
    ASSERT(buf.eof());

}


// ################################################################
//                              edge
// ################################################################

// ######################
// CMD_GET_EDGE_VARIABLE
// ######################

list<string> TraCI_Extend::edgeGetIDList()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_EDGE_VARIABLE, "", ID_LIST, RESPONSE_GET_EDGE_VARIABLE);
}


uint32_t TraCI_Extend::edgeGetIDCount()
{
    return getCommandInterface()->genericGetInt(CMD_GET_EDGE_VARIABLE, "", ID_COUNT, RESPONSE_GET_EDGE_VARIABLE);
}


double TraCI_Extend::edgeGetMeanTravelTime(string Id)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_EDGE_VARIABLE, Id, VAR_CURRENT_TRAVELTIME, RESPONSE_GET_EDGE_VARIABLE);
}


uint32_t TraCI_Extend::edgeGetLastStepVehicleNumber(string Id)
{
    return getCommandInterface()->genericGetInt(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_VEHICLE_NUMBER, RESPONSE_GET_EDGE_VARIABLE);
}


list<string> TraCI_Extend::edgeGetLastStepVehicleIDs(string Id)
{
    return getCommandInterface()->genericGetStringList(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_EDGE_VARIABLE);
}


double TraCI_Extend::edgeGetLastStepMeanVehicleSpeed(string Id)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_MEAN_SPEED, RESPONSE_GET_EDGE_VARIABLE);
}


double TraCI_Extend::edgeGetLastStepMeanVehicleLength(string Id)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_EDGE_VARIABLE, Id, LAST_STEP_LENGTH, RESPONSE_GET_EDGE_VARIABLE);
}


list<string> TraCI_Extend::edgeGetLastStepPersonIDs(string Id)
{
    return getCommandInterface()->genericGetStringList(CMD_GET_EDGE_VARIABLE, Id, 0x1a, RESPONSE_GET_EDGE_VARIABLE);
}


// ######################
// CMD_SET_EDGE_VARIABLE
// ######################

void TraCI_Extend::edgeSetGlobalTravelTime(string edgeId, int32_t beginT, int32_t endT, double value)
{
    uint8_t variableId = VAR_EDGE_TRAVELTIME;
    uint8_t variableType = TYPE_COMPOUND;
    int32_t count = 3;
    uint8_t valueI = TYPE_INTEGER;
    uint8_t valueD = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_EDGE_VARIABLE, TraCIBuffer() << variableId << edgeId
                                                                                    << variableType << count
                                                                                    << valueI << beginT
                                                                                    << valueI << endT
                                                                                    << valueD << value
                                                                                    );
    ASSERT(buf.eof());
}


// ################################################################
//                              lane
// ################################################################

// #####################
// CMD_GET_LANE_VARIABLE
// #####################

// gets a list of all lanes in the network
list<string> TraCI_Extend::laneGetIDList()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_LANE_VARIABLE, "", ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}


uint32_t TraCI_Extend::laneGetIDCount()
{
    return getCommandInterface()->genericGetInt(CMD_GET_LANE_VARIABLE, "", ID_COUNT, RESPONSE_GET_LANE_VARIABLE);
}


string TraCI_Extend::laneGetEdgeID(string laneId)
{
    return getCommandInterface()->genericGetString(CMD_GET_LANE_VARIABLE, laneId, LANE_EDGE_ID, RESPONSE_GET_LANE_VARIABLE);
}


double TraCI_Extend::laneGetLength(string laneId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_LENGTH, RESPONSE_GET_LANE_VARIABLE);
}


double TraCI_Extend::laneGetMaxSpeed(string laneId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_MAXSPEED, RESPONSE_GET_LANE_VARIABLE);
}


uint32_t TraCI_Extend::laneGetLastStepVehicleNumber(string laneId)
{
    return getCommandInterface()->genericGetInt(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_VEHICLE_NUMBER, RESPONSE_GET_LANE_VARIABLE);
}


list<string> TraCI_Extend::laneGetLastStepVehicleIDs(string laneId)
{
    return getCommandInterface()->genericGetStringList(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}


double TraCI_Extend::laneGetLastStepMeanVehicleSpeed(string laneId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_MEAN_SPEED, RESPONSE_GET_LANE_VARIABLE);
}


double TraCI_Extend::laneGetLastStepMeanVehicleLength(string laneId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_LENGTH, RESPONSE_GET_LANE_VARIABLE);
}


// ######################
// CMD_SET_LANE_VARIABLE
// ######################

void TraCI_Extend::laneSetMaxSpeed(string laneId, double value)
{
    uint8_t variableId = VAR_MAXSPEED;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_LANE_VARIABLE, TraCIBuffer() << variableId << laneId << variableType << value);
    ASSERT(buf.eof());
}



// ################################################################
//                 loop detector (E1-Detectors)
// ################################################################

// ###############################
// CMD_GET_INDUCTIONLOOP_VARIABLE
// ###############################

list<string> TraCI_Extend::LDGetIDList()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_INDUCTIONLOOP_VARIABLE, "", 0x00, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


uint32_t TraCI_Extend::LDGetIDCount(string loopId)
{
    return getCommandInterface()->genericGetInt(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x01, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


string TraCI_Extend::LDGetLaneID(string loopId)
{
    return getCommandInterface()->genericGetString(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x51, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


double TraCI_Extend::LDGetPosition(string loopId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x42, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


uint32_t TraCI_Extend::LDGetLastStepVehicleNumber(string loopId)
{
    return getCommandInterface()->genericGetInt(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x10, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


list<string> TraCI_Extend::LDGetLastStepVehicleIDs(string loopId)
{
    return getCommandInterface()->genericGetStringList(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x12, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


double TraCI_Extend::LDGetLastStepMeanVehicleSpeed(string loopId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x11, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


double TraCI_Extend::LDGetElapsedTimeLastDetection(string loopId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x16, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


vector<string> TraCI_Extend::LDGetLastStepVehicleData(string loopId)
{
    uint8_t resultTypeId = TYPE_COMPOUND;   // note: type is compound!
    uint8_t variableId = 0x17;
    uint8_t responseId = RESPONSE_GET_INDUCTIONLOOP_VARIABLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_GET_INDUCTIONLOOP_VARIABLE, TraCIBuffer() << variableId << loopId);

    uint8_t cmdLength; buf >> cmdLength;
    if (cmdLength == 0) {
        uint32_t cmdLengthX;
        buf >> cmdLengthX;
    }
    uint8_t commandId_r; buf >> commandId_r;
    ASSERT(commandId_r == responseId);
    uint8_t varId; buf >> varId;
    ASSERT(varId == variableId);
    string objectId_r; buf >> objectId_r;
    ASSERT(objectId_r == loopId);

    uint8_t resType_r; buf >> resType_r;
    ASSERT(resType_r == resultTypeId);
    uint32_t count; buf >> count;

    // now we start getting real data that we are looking for
    vector<string> res;
    ostringstream strs;

    // number of information packets (in 'No' variable)
    uint8_t typeI; buf >> typeI;
    uint32_t No; buf >> No;

    for (uint32_t i = 1; i <= No; i++)
    {
        // get vehicle id
        uint8_t typeS1; buf >> typeS1;
        string vId; buf >> vId;
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
        string vehicleTypeID; buf >> vehicleTypeID;
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

list<string> TraCI_Extend::LADGetIDList()
{
    return getCommandInterface()->genericGetStringList(0xad, "", 0x00, 0xbd);
}


uint32_t TraCI_Extend::LADGetIDCount(string loopId)
{
    return getCommandInterface()->genericGetInt(0xad, loopId, 0x01, 0xbd);
}


uint32_t TraCI_Extend::LADGetLastStepVehicleNumber(string loopId)
{
    return getCommandInterface()->genericGetInt(0xad, loopId, 0x10, 0xbd);
}


list<string> TraCI_Extend::LADGetLastStepVehicleIDs(string loopId)
{
    return getCommandInterface()->genericGetStringList(0xad, loopId, 0x12, 0xbd);
}


double TraCI_Extend::LADGetLastStepMeanVehicleSpeed(string loopId)
{
    return getCommandInterface()->genericGetDouble(0xad, loopId, 0x11, 0xbd);
}



// ################################################################
//                          traffic light
// ################################################################

// ###################
// CMD_GET_TL_VARIABLE
// ###################

list<string> TraCI_Extend::TLGetIDList()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_TL_VARIABLE, "",ID_LIST, RESPONSE_GET_TL_VARIABLE);
}


uint32_t TraCI_Extend::TLGetIDCount()
{
    return getCommandInterface()->genericGetInt(CMD_GET_TL_VARIABLE, "",ID_COUNT, RESPONSE_GET_TL_VARIABLE);
}


list<string> TraCI_Extend::TLGetControlledLanes(string TLid)
{
    return getCommandInterface()->genericGetStringList(CMD_GET_TL_VARIABLE, TLid, TL_CONTROLLED_LANES, RESPONSE_GET_TL_VARIABLE);
}


map<int,string> TraCI_Extend::TLGetControlledLinks(string TLid)
{
    uint8_t resultTypeId = TYPE_COMPOUND;
    uint8_t variableId = TL_CONTROLLED_LINKS;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_GET_TL_VARIABLE, TraCIBuffer() << variableId << TLid);

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

    map<int,string> myMap;

    for (uint32_t i = 0; i < No; i++)
    {
        buf >> typeI;
        uint32_t No2; buf >> No2;

        buf >> typeI;
        uint32_t No3; buf >> No3;

        string lanesForThisLink = "";
        for(uint32_t j = 0; j < No3; j++)
        {
            std::string id; buf >> id;
            lanesForThisLink = lanesForThisLink + id + "|";
        }

        myMap[i] = lanesForThisLink;
    }

    return myMap;
}


string TraCI_Extend::TLGetProgram(string TLid)
{
    return getCommandInterface()->genericGetString(CMD_GET_TL_VARIABLE, TLid, TL_CURRENT_PROGRAM, RESPONSE_GET_TL_VARIABLE);
}


uint32_t TraCI_Extend::TLGetPhase(string TLid)
{
    return getCommandInterface()->genericGetInt(CMD_GET_TL_VARIABLE, TLid, TL_CURRENT_PHASE, RESPONSE_GET_TL_VARIABLE);
}


string TraCI_Extend::TLGetState(string TLid)
{
    return getCommandInterface()->genericGetString(CMD_GET_TL_VARIABLE, TLid, TL_RED_YELLOW_GREEN_STATE, RESPONSE_GET_TL_VARIABLE);
}


uint32_t TraCI_Extend::TLGetPhaseDuration(string TLid)
{
    return getCommandInterface()->genericGetInt(CMD_GET_TL_VARIABLE, TLid, TL_PHASE_DURATION, RESPONSE_GET_TL_VARIABLE);
}


uint32_t TraCI_Extend::TLGetNextSwitchTime(string TLid)
{
    return getCommandInterface()->genericGetInt(CMD_GET_TL_VARIABLE, TLid, TL_NEXT_SWITCH, RESPONSE_GET_TL_VARIABLE);
}


// ###################
// CMD_SET_TL_VARIABLE
// ###################

void TraCI_Extend::TLSetProgram(string TLid, string value)
{
    uint8_t variableId = TL_PROGRAM;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::TLSetPhaseIndex(string TLid, int value)
{
    uint8_t variableId = TL_PHASE_INDEX;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::TLSetPhaseDuration(string TLid, int value)
{
    uint8_t variableId = TL_PHASE_DURATION;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::TLSetState(string TLid, string value)
{
    uint8_t variableId = TL_RED_YELLOW_GREEN_STATE;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());
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


vector<double> TraCI_Extend::GUIGetBoundry()
{
    return genericGetBoundingBox(CMD_GET_GUI_VARIABLE, "View #0", VAR_VIEW_BOUNDARY, RESPONSE_GET_GUI_VARIABLE);
}


// #####################
// CMD_SET_GUI_VARIABLE
// #####################

void TraCI_Extend::GUISetZoom(double value)
{
    uint8_t variableId = VAR_VIEW_ZOOM;
    string viewID = "View #0";
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::GUISetOffset(double x, double y)
{
    uint8_t variableId = VAR_VIEW_OFFSET;
    string viewID = "View #0";
    uint8_t variableType = POSITION_2D;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << x << y);
    ASSERT(buf.eof());
}


// very slow!
void TraCI_Extend::GUISetTrackVehicle(string nodeId)
{
    uint8_t variableId = VAR_TRACK_VEHICLE;
    string viewID = "View #0";
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << nodeId);
    ASSERT(buf.eof());
}


// ################################################################
//                               polygon
// ################################################################

// ########################
// CMD_GET_POLYGON_VARIABLE
// ########################

list<string> TraCI_Extend::polygonGetIDList()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_POLYGON_VARIABLE, "", ID_LIST, RESPONSE_GET_POLYGON_VARIABLE);
}


uint32_t TraCI_Extend::polygonGetIDCount()
{
    return getCommandInterface()->genericGetInt(CMD_GET_POLYGON_VARIABLE, "",ID_COUNT, RESPONSE_GET_POLYGON_VARIABLE);
}


std::list<Coord> TraCI_Extend::polygonGetShape(string polyId)
{
    return getCommandInterface()->genericGetCoordList(CMD_GET_POLYGON_VARIABLE, polyId, VAR_SHAPE, RESPONSE_GET_POLYGON_VARIABLE);
}


// ########################
// CMD_SET_POLYGON_VARIABLE
// ########################

void TraCI_Extend::polygonAdd(std::string polyId, std::string polyType, const TraCIColor& color, bool filled, int32_t layer, const std::list<TraCICoord>& points)
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

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_POLYGON_VARIABLE, p);
    ASSERT(buf.eof());
}


void TraCI_Extend::polygonSetFilled(string polyId, uint8_t filled)
{
    uint8_t variableId = VAR_FILL;
    uint8_t variableType = TYPE_UBYTE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_POLYGON_VARIABLE, TraCIBuffer() << variableId << polyId << variableType << filled);
    ASSERT(buf.eof());
}


// ################################################################
//                               person
// ################################################################

// ##############
// CMD_GET_PERSON
// ##############

list<string> TraCI_Extend::personGetIDList()
{
    return getCommandInterface()->genericGetStringList(0xae, "", ID_LIST, 0xbe);
}


uint32_t TraCI_Extend::personGetIDCount()
{
    return getCommandInterface()->genericGetInt(0xae, "", ID_COUNT, 0xbe);
}


string TraCI_Extend::personGetTypeID(string pId)
{
    return getCommandInterface()->genericGetString(0xae, pId, VAR_TYPE, 0xbe);
}


Coord TraCI_Extend::personGetPosition(string pId)
{
    return genericGetCoordv2(0xae, pId, VAR_POSITION, 0xbe);
}


string TraCI_Extend::personGetEdgeID(string pId)
{
    return getCommandInterface()->genericGetString(0xae, pId, VAR_ROAD_ID, 0xbe);
}


double TraCI_Extend::personGetEdgePosition(string pId)
{
    return getCommandInterface()->genericGetDouble(0xae, pId, 0x56, 0xbe);
}


double TraCI_Extend::personGetSpeed(string pId)
{
    return getCommandInterface()->genericGetDouble(0xae, pId, VAR_SPEED, 0xbe);
}


string TraCI_Extend::personGetNextEdge(string pId)
{
    return getCommandInterface()->genericGetString(0xae, pId, 0xc1, 0xbe);
}

}

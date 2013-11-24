
#include "TraCI_Extend.h"

#include <sstream>
#include <iostream>
#include <fstream>

Define_Module(TraCI_Extend);


TraCI_Extend::~TraCI_Extend()
{

}


void TraCI_Extend::initialize(int stage)
{
    TraCIScenarioManagerLaunchd::initialize(stage);

    updataGUI = new cMessage("updataGUI", 1);

    tracking = par("tracking").boolValue();
    trackingV = par("trackingV").stdstringValue();
    trackingInterval = par("trackingInterval").doubleValue();
}


void TraCI_Extend::handleSelfMsg(cMessage *msg)
{
    if (msg == updataGUI)
    {
        Coord co = commandGetVehiclePos(trackingV);
        commandSetGUIOffset(co.x, co.y);

        scheduleAt(simTime() + trackingInterval, updataGUI);
    }
    else
    {
        TraCIScenarioManager::handleSelfMsg(msg);
    }
}


void TraCI_Extend::init_traci()
{
    TraCIScenarioManagerLaunchd::init_traci();

    // zoom-in GUI
    commandSetGUIZoom(767.);

    // adjust windows
    commandSetGUIOffset(200., 0.);

    // track the vehicle only if tracking is on
    if(tracking)
    {
        scheduleAt(simTime(), updataGUI);
    }
}


// #################################
// getter methods added to the veins
// #################################

uint32_t TraCI_Extend::commandGetNoVehicles()
{
    return genericGetInt32(CMD_GET_VEHICLE_VARIABLE, "", ID_COUNT, RESPONSE_GET_VEHICLE_VARIABLE);
}


// gets a list of all vehicles in the network (alphabetically!!!)
std::list<std::string> TraCI_Extend::commandGetVehicleList()
{
    return genericGetStringList(CMD_GET_VEHICLE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLE_VARIABLE);
}


// gets a list of all lanes in the network
std::list<std::string> TraCI_Extend::commandGetLaneList()
{
    return genericGetStringList(CMD_GET_LANE_VARIABLE, "", ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}


// gets a list of all lanes in the network
std::list<std::string> TraCI_Extend::commandGetVehicleLaneList(std::string edgeId)
{
    return genericGetStringList(CMD_GET_LANE_VARIABLE, edgeId, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}


double TraCI_Extend::commandGetVehicleSpeed(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_SPEED, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::commandGetVehicleAccel(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x41, RESPONSE_GET_VEHICLE_VARIABLE);
}


std::string TraCI_Extend::commandGetVehicleType(std::string nodeId)
{
    return genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TYPE, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::commandGetVehicleLength(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, VAR_LENGTH, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


double TraCI_Extend::commandGetVehicleMaxDecel(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, VAR_DECEL, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


std::list<std::string> TraCI_Extend::commandGetVehiclesOnLane(std::string laneId)
{
    return genericGetStringList(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}


Coord TraCI_Extend::commandGetVehiclePos(std::string nodeId)
{
    return genericGetCoordv2(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_POSITION, RESPONSE_GET_VEHICLE_VARIABLE);
}


void TraCI_Extend::commandTerminate()
{
        TraCIBuffer buf = queryTraCI(CMD_CLOSE, TraCIBuffer() << 0);

        uint32_t count;
        buf >> count;
}


uint32_t TraCI_Extend::genericGetInt32(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
{
    uint8_t resultTypeId = TYPE_INTEGER;
    uint32_t res;

    TraCIBuffer buf = TraCIScenarioManager::queryTraCI(commandId, TraCIBuffer() << variableId << objectId);

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

    TraCIBuffer buf = queryTraCI(commandId, TraCIBuffer() << variableId << objectId);

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

    return Coord(x, y);
}

// #################################
// setter methods added to the veins
// #################################

// todo: remove this?
void TraCI_Extend::commandSetMaxAccel(std::string nodeId, double value)
{
    uint8_t variableId = VAR_ACCEL;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


// todo: remove this?
void TraCI_Extend::commandSetMaxDecel(std::string nodeId, double value)
{
    uint8_t variableId = VAR_DECEL;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetPreceding(std::string nodeId, std::string value)
{
    uint8_t variableId = 0x15;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetPlatoonLeader(std::string nodeId, std::string value)
{
    uint8_t variableId = 0x16;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetModeSwitch(std::string nodeId, bool value)
{
    uint8_t variableId = 0x17;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << (int)value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetGUIZoom(double value)
{
    uint8_t variableId = VAR_VIEW_ZOOM;
    std::string viewID = "View #0";
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = queryTraCI(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << value);
    ASSERT(buf.eof());
}


// very slow!
void TraCI_Extend::commandSetGUITrack(std::string nodeId)
{
    uint8_t variableId = VAR_TRACK_VEHICLE;
    std::string viewID = "View #0";
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = queryTraCI(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << nodeId);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetGUIOffset(double x, double y)
{
    uint8_t variableId = VAR_VIEW_OFFSET;
    std::string viewID = "View #0";
    uint8_t variableType = POSITION_2D;

    TraCIBuffer buf = queryTraCI(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << x << y);
    ASSERT(buf.eof());
}


void TraCI_Extend::finish()
{
    TraCIScenarioManagerLaunchd::finish();
}


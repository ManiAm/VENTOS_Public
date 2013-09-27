
#include "MyTraCI.h"

#include <sstream>
#include <iostream>
#include <fstream>

#define MYDEBUG EV

Define_Module(MyTraCI);

MyTraCI::~MyTraCI()
{

}


void MyTraCI::initialize(int stage)
{
    TraCIScenarioManagerLaunchd::initialize(stage);

    if(stage == 1)
    {
        // todo: change file name to the CFModel typeid(veh.getCarFollowModel()).name()
        char fName [50];
        sprintf (fName, "%s.txt", "results/speed-gap");

        f1 = fopen (fName, "w");

        fprintf (f1, "%-10s","vehicle");
        fprintf (f1, "%-12s","timeStep");
        fprintf (f1, "%-10s","speed");
        fprintf (f1, "%-12s","accel");
        fprintf (f1, "%-12s","pos");
        fprintf (f1, "%-10s","gap");
        fprintf (f1, "%-10s\n\n","timeGap");

        fflush(f1);
    }
}


void MyTraCI::init_traci()
{
    TraCIScenarioManagerLaunchd::init_traci();

    {
        // query road network boundaries
        TraCIBuffer buf = queryTraCI(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_NET_BOUNDING_BOX) << std::string("sim0"));
        uint8_t cmdLength_resp;
        buf >> cmdLength_resp;

        uint8_t commandId_resp;
        buf >> commandId_resp;
        ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);

        uint8_t variableId_resp;
        buf >> variableId_resp;
        ASSERT(variableId_resp == VAR_NET_BOUNDING_BOX);

        std::string simId;
        buf >> simId;

        uint8_t typeId_resp;
        buf >> typeId_resp;
        ASSERT(typeId_resp == TYPE_BOUNDINGBOX);

        double x1; buf >> x1;
        double y1; buf >> y1;
        double x2; buf >> x2;
        double y2; buf >> y2;

        ASSERT(buf.eof());

        netbounds1 = TraCICoord(x1, y1);
        netbounds2 = TraCICoord(x2, y2);

        MYDEBUG << "Default playground size is " << world->getPgs()->x << " by " << world->getPgs()->y << endl;
        MYDEBUG << "Change into " << traci2omnet(netbounds2).x << " by " << traci2omnet(netbounds1).y << endl;
    }
}


void MyTraCI::executeOneTimestep()
{
    TraCIScenarioManager::executeOneTimestep();

    // Now we write the result into file

    uint32_t vehicles = commandGetNoVehicles();
    if(vehicles == 0)
        return;

    // get a list of all vehicles in the network
    std::list<std::string> list = commandGetVehicleList();

    std::string vleaderID = "";

    for(std::list<std::string>::reverse_iterator i = list.rbegin(); i != list.rend(); ++i)
    {
        std::string vID = i->c_str();
        writeToFile(vID, vleaderID);
        vleaderID = i->c_str();
    }

    // change the speed of manual driving vehicle
    AccelDecelManual();
}


void MyTraCI::AccelDecelManual()
{
    if( simTime().dbl() == 30 )
    {
        commandSetSpeed(std::string("Manual"), (double) 2.0);
    }
    else if(simTime().dbl() == 50)
    {
        commandSetSpeed(std::string("Manual"),(double) 20.0);
    }
    else if(simTime().dbl() == 70)
    {
        commandSetSpeed(std::string("Manual"),(double) 2.0);
    }
    else if(simTime().dbl() == 90)
    {
        commandSetSpeed(std::string("Manual"),(double) 20.0);
    }
}


// vID is current vehicle, vleaderID is the leader (if present)
void MyTraCI::writeToFile(std::string vID, std::string vleaderID)
{
    double speed = commandGetVehicleSpeed(vID);
    double pos = commandGetLanePosition(vID);

    // calculate gap (if leading is present)
    double gap = -1;

    if(vleaderID != "")
    {
        std::string leaderType = commandGetVehicleType(vleaderID);
        gap = commandGetLanePosition(vleaderID) - commandGetLanePosition(vID) - commandGetVehicleLength(leaderType);
    }

    // calculate timeGap (if leading is present)
    double timeGap = -1;

    if(vleaderID != "" && speed != 0)
        timeGap = gap / speed;

    // write the vehicle data into file
    fprintf (f1, "%-10s ", vID.c_str());
    fprintf (f1, "%-10.2f ", (simTime()-updateInterval).dbl());
    fprintf (f1, "%-10.2f ", speed);
    fprintf (f1, "%-10.2f ", -2.0); // todo: acceleration
    fprintf (f1, "%-10.2f ", pos);
    fprintf (f1, "%-10.2f ", gap);
    fprintf (f1, "%-10.2f \n", timeGap);

    fflush(f1);
}


uint32_t MyTraCI::commandGetNoVehicles()
{
    return genericGetInt32(CMD_GET_VEHICLE_VARIABLE, "", ID_COUNT, RESPONSE_GET_VEHICLE_VARIABLE);
}


std::list<std::string> MyTraCI::commandGetVehicleList()
{
    return genericGetStringList(CMD_GET_VEHICLE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLE_VARIABLE);
}


double MyTraCI::commandGetVehicleSpeed(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_SPEED, RESPONSE_GET_VEHICLE_VARIABLE);
}


std::string MyTraCI::commandGetVehicleType(std::string nodeId)
{
    return genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TYPE, RESPONSE_GET_VEHICLE_VARIABLE);
}


double MyTraCI::commandGetVehicleLength(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, VAR_LENGTH, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


double MyTraCI::commandGetVehicleMaxDecel(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, VAR_DECEL, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


std::list<std::string> MyTraCI::commandGetVehiclesOnLane(std::string laneId)
{
    return genericGetStringList(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}


Coord MyTraCI::commandGetVehiclePos(std::string nodeId)
{
    return genericGetCoordv2(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_POSITION, RESPONSE_GET_VEHICLE_VARIABLE);
}


uint32_t MyTraCI::genericGetInt32(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
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


// the same as genericGetCoordv, but no conversion to omnet++ coordinates at the end
Coord MyTraCI::genericGetCoordv2(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId)
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


void MyTraCI::finish()
{
    TraCIScenarioManagerLaunchd::finish();
    fclose(f1);
}


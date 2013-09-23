
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


void MyTraCI::executeOneTimestep()
{
    // change the speed of manual driving vehicle
    AccelDecelManual();

    TraCIScenarioManager::executeOneTimestep();

    // SUMO simulation has advances 1 TS
    // Now we write the result into file

    uint32_t vehicles = commandGetNoVehicles();
    if(vehicles == 0)
        return;

    std::list<std::string> list = commandGetVehicleList();

    std::string vleaderID = "";

    for(std::list<std::string>::reverse_iterator i = list.rbegin(); i != list.rend(); ++i)
    {
        std::string vID = i->c_str();
        writeToFile(vID, vleaderID);
        vleaderID = i->c_str();
    }
}


void MyTraCI::AccelDecelManual()
{


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


void MyTraCI::finish()
{
    TraCIScenarioManagerLaunchd::finish();
    fclose(f1);
}


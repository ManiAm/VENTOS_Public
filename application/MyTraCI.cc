
#include "MyTraCI.h"

#include <sstream>
#include <iostream>
#include <fstream>

#define MYDEBUG EV

Define_Module(MyTraCI);

int MyTraCI::index = 1;
int MyTraCI::fileLine = 0;
bool MyTraCI::endOfFile = false;


MyTraCI::~MyTraCI()
{

}


void MyTraCI::initialize(int stage)
{
    TraCIScenarioManagerLaunchd::initialize(stage);

    if(stage ==0)
    {
        trajectoryMode = par("trajectoryMode").longValue();
        trajectory = par("trajectory").stringValue();

        f2 = fopen ("sumo/EX_Trajectory.txt", "r");

        if ( f2 == NULL )
            error("external trajectory file does not exists!");
    }

}


void MyTraCI::init_traci()
{
    TraCIScenarioManagerLaunchd::init_traci();

    // making sure that platoonLeader exists in the sumo


    // send command to sumo-GUI


    // ---------------------------------------------
    char fName [50];
    sprintf (fName, "%s.txt", "results/speed-gap");

    f1 = fopen (fName, "w");

    // write global information
    fprintf (f1, "Number of vehicles: %d \n\n", commandGetNoVehicles());

    // write header
    fprintf (f1, "%-10s","index");
    fprintf (f1, "%-10s","vehicle");
    fprintf (f1, "%-12s","timeStep");
    fprintf (f1, "%-10s","speed");
    fprintf (f1, "%-12s","accel");
    fprintf (f1, "%-12s","pos");
    fprintf (f1, "%-10s","gap");
    fprintf (f1, "%-10s\n\n","timeGap");

    fflush(f1);
}


void MyTraCI::executeOneTimestep()
{
    TraCIScenarioManager::executeOneTimestep();

    EV << "### Sumo advanced to " << getCurrentTimeMs() / 1000. << std::endl;

    // Now we write the result into file
    writeToFile();

    if(trajectoryMode == 0)
    {
        // no trajectory
    }
    // trajectory accel/decel at specific times
    else if(trajectoryMode == 1)
    {
        AccelDecel();
    }
    // trajectory accel/decel based on external file
    else if(trajectoryMode == 2)
    {
        ExTrajectory();
    }
}


void MyTraCI::writeToFile()
{
    std::string vleaderID = "";

    // get all lanes in the network
    std::list<std::string> list = commandGetLaneList();

    for(std::list<std::string>::iterator i = list.begin(); i != list.end(); ++i)
    {
        // get all vehicles on lane i
        std::list<std::string> list2 = commandGetVehicleLaneList( i->c_str() );

        for(std::list<std::string>::reverse_iterator k = list2.rbegin(); k != list2.rend(); ++k)
        {
            std::string vID = k->c_str();
            writeToFilePerVehicle(vID, vleaderID);
            vleaderID = k->c_str();
        }
    }

    // increase index after writing data for all vehicles
    if (commandGetNoVehicles() > 0)
        index++;
}


// vID is current vehicle, vleaderID is the leader (if present)
void MyTraCI::writeToFilePerVehicle(std::string vID, std::string vleaderID)
{
    double speed = commandGetVehicleSpeed(vID);
    double pos = commandGetLanePosition(vID);
    double accel = commandGetVehicleAccel(vID);

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

    // write the current vehicle data into file
    fprintf (f1, "%-10d ", index);
    fprintf (f1, "%-10s ", vID.c_str());
    fprintf (f1, "%-10.2f ", ( simTime()-updateInterval).dbl() );
    fprintf (f1, "%-10.2f ", speed);
    fprintf (f1, "%-10.2f ", accel);
    fprintf (f1, "%-10.2f ", pos);
    fprintf (f1, "%-10.2f ", gap);
    fprintf (f1, "%-10.2f \n", timeGap);

    fflush(f1);
}


void MyTraCI::AccelDecel()
{
    if( simTime().dbl() == 30 )
    {
        commandSetSpeed(trajectory, (double) 0);
    }
    else if(simTime().dbl() == 70)
    {
        commandSetSpeed(trajectory,(double) 20.0);
    }
    else if(simTime().dbl() == 120)
    {
        commandSetSpeed(trajectory,(double) 0);
    }
    else if(simTime().dbl() == 170)
    {
        commandSetSpeed(trajectory,(double) 20.0);
    }
}


void MyTraCI::ExTrajectory()
{
    if(simTime().dbl() < 30)
    {
        return;
    }
    else if(simTime().dbl() == 30)
    {
        commandSetSpeed(trajectory, (double) 0);
        return;
    }
    else if(simTime().dbl() < 70)
    {
        return;
    }

    if(endOfFile)
        return;

    fileLine++;  // a static variable
    char line [128];  // maximum line size
    double speed = 0;

    char *result = fgets (line, sizeof line, f2);

    if (result == NULL)
    {
        endOfFile = true;
        return;
    }

    speed = atof(line);

    commandSetSpeed(trajectory, speed);



    //EV << "### line " << fileLine << ": " << speed << std::endl;
}


// ##################################
// getter methods added to the veins
// ##################################

uint32_t MyTraCI::commandGetNoVehicles()
{
    return genericGetInt32(CMD_GET_VEHICLE_VARIABLE, "", ID_COUNT, RESPONSE_GET_VEHICLE_VARIABLE);
}


// gets a list of all vehicles in the network (alphabetically!!!)
std::list<std::string> MyTraCI::commandGetVehicleList()
{
    return genericGetStringList(CMD_GET_VEHICLE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLE_VARIABLE);
}


// gets a list of all lanes in the network
std::list<std::string> MyTraCI::commandGetLaneList()
{
    return genericGetStringList(CMD_GET_LANE_VARIABLE, "", ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}


// gets a list of all lanes in the network
std::list<std::string> MyTraCI::commandGetVehicleLaneList(std::string edgeId)
{
    return genericGetStringList(CMD_GET_LANE_VARIABLE, edgeId, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}


double MyTraCI::commandGetVehicleSpeed(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_SPEED, RESPONSE_GET_VEHICLE_VARIABLE);
}


double MyTraCI::commandGetVehicleAccel(std::string nodeId)
{
    return genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x41, RESPONSE_GET_VEHICLE_VARIABLE);
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


// same as genericGetCoordv, but no conversion to omnet++ coordinates at the end
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

// ##################################
// setter methods added to the veins
// ##################################

void MyTraCI::commandSetPreceding(std::string nodeId, std::string value)
{
    uint8_t variableId = 0x15;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void MyTraCI::commandSetPlatoonLeader(std::string nodeId, std::string value)
{
    uint8_t variableId = 0x16;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void MyTraCI::commandSetModeSwitch(std::string nodeId, bool value)
{
    uint8_t variableId = 0x17;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = queryTraCI(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << (int)value);
    ASSERT(buf.eof());
}


void MyTraCI::finish()
{
    TraCIScenarioManagerLaunchd::finish();

    fclose(f1);
    fclose(f2);
}



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

    if (apiVersionS == 9) {
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


// ####################
// CMD_GET_SIM_VARIABLE
// ####################

double* TraCI_Extend::commandGetNetworkBoundary()
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


uint32_t TraCI_Extend::commandGetMinExpectedVehicles()
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


uint32_t TraCI_Extend::commandGetArrivedVehiclesCount()
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

void TraCI_Extend::commandTerminate()
{
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_CLOSE, TraCIBuffer() << 0);

    uint32_t count;
    buf >> count;
}


// #########################
// CMD_GET_VEHICLE_VARIABLE
// #########################

// gets a list of all vehicles in the network (alphabetically!!!)
list<string> TraCI_Extend::commandGetVehicleList()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_VEHICLE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLE_VARIABLE);
}


uint32_t TraCI_Extend::commandGetVehicleCount()
{
    return getCommandInterface()->genericGetInt(CMD_GET_VEHICLE_VARIABLE, "", ID_COUNT, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::commandGetVehicleSpeed(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_SPEED, RESPONSE_GET_VEHICLE_VARIABLE);
}


Coord TraCI_Extend::commandGetVehiclePos(string nodeId)
{
    return genericGetCoordv2(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_POSITION, RESPONSE_GET_VEHICLE_VARIABLE);
}


string TraCI_Extend::commandGetVehicleEdgeId(std::string nodeId)
{
    return getCommandInterface()->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROAD_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}


std::string TraCI_Extend::commandGetVehicleLaneId(string nodeId)
{
    return getCommandInterface()->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}


uint32_t TraCI_Extend::commandGetVehicleLaneIndex(string nodeId)
{
    return getCommandInterface()->genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_INDEX, RESPONSE_GET_VEHICLE_VARIABLE);
}


list<string> TraCI_Extend::commandGetVehicleEdgeList(string nodeId)
{
    return getCommandInterface()->genericGetStringList(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_EDGES, RESPONSE_GET_VEHICLE_VARIABLE);
}


string TraCI_Extend::commandGetVehicleTypeId(string nodeId)
{
    return getCommandInterface()->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TYPE, RESPONSE_GET_VEHICLE_VARIABLE);
}


uint8_t* TraCI_Extend::commandGetVehicleColor(string nodeId)
{
    return genericGetArrayUnsignedInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_COLOR, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::commandGetVehicleLanePosition(std::string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANEPOSITION, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::commandGetVehicleLength(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LENGTH, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::commandGetVehicleMaxDecel(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_DECEL, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::commandGetVehicleTimeGap(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TAU, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::commandGetVehicleMinGap(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_MINGAP, RESPONSE_GET_VEHICLE_VARIABLE);
}


// my own method! [deprecated!]
string TraCI_Extend::commandGetLeadingVehicle_old(string nodeId)
{
    // get the lane id (like 1to2_0)
    string laneID = commandGetVehicleLaneId(nodeId);

    // get a list of all vehicles on this lane (left to right)
    list<string> myList = commandGetLaneVehicleList(laneID);

    string vleaderID = "";

    for(list<string>::reverse_iterator i = myList.rbegin(); i != myList.rend(); ++i)
    {
        string currentID = i->c_str();

        if(currentID == nodeId)
            return vleaderID;

        vleaderID = i->c_str();
    }

    return "";
}


vector<string> TraCI_Extend::commandGetLeadingVehicle(string nodeId, double look_ahead_distance)
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
    len = len + commandGetVehicleMinGap(nodeId);

    // now convert len to string
    ostringstream s;
    s << len;
    res.push_back(s.str());

    ASSERT(buf.eof());

    return res;
}


double TraCI_Extend::commandGetVehicleAccel(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x70, RESPONSE_GET_VEHICLE_VARIABLE);
}


int TraCI_Extend::commandGetVehicleCFMode(string nodeId)
{
    return getCommandInterface()->genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x71, RESPONSE_GET_VEHICLE_VARIABLE);
}


// ############################
// CMD_GET_VEHICLETYPE_VARIABLE
// ############################

double TraCI_Extend::commandGetVehicleTypeLength(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, VAR_LENGTH, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


int TraCI_Extend::commandGetVehicleControllerType(string nodeId)
{
    return getCommandInterface()->genericGetInt(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, 0x02, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


int TraCI_Extend::commandGetVehicleControllerNumber(string nodeId)
{
    return getCommandInterface()->genericGetInt(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, 0x03, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


// ######################
// CMD_GET_ROUTE_VARIABLE
// ######################

list<string> TraCI_Extend::commandGetRouteIds()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_ROUTE_VARIABLE, "", 0x00, RESPONSE_GET_ROUTE_VARIABLE);
}


// #####################
// CMD_GET_LANE_VARIABLE
// #####################

// gets a list of all lanes in the network
list<string> TraCI_Extend::commandGetLaneList()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_LANE_VARIABLE, "", ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}


list<string> TraCI_Extend::commandGetLaneVehicleList(string edgeId)
{
    return getCommandInterface()->genericGetStringList(CMD_GET_LANE_VARIABLE, edgeId, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}


double TraCI_Extend::commandGetLaneLength(std::string laneId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_LENGTH, RESPONSE_GET_LANE_VARIABLE);
}


double TraCI_Extend::commandGetLaneMeanSpeed(std::string laneId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_MEAN_SPEED, RESPONSE_GET_LANE_VARIABLE);
}



// ###############################
// CMD_GET_INDUCTIONLOOP_VARIABLE
// ###############################

list<string> TraCI_Extend::commandGetLoopDetectorList()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_INDUCTIONLOOP_VARIABLE, "", 0x00, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


uint32_t TraCI_Extend::commandGetLoopDetectorCount(string loopId)
{
    return getCommandInterface()->genericGetInt(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x10, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


double TraCI_Extend::commandGetLoopDetectorSpeed(string loopId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x11, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


list<string> TraCI_Extend::commandGetLoopDetectorVehicleList(string loopId)
{
    return getCommandInterface()->genericGetStringList(CMD_GET_INDUCTIONLOOP_VARIABLE, loopId, 0x12, RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
}


vector<string> TraCI_Extend::commandGetLoopDetectorVehicleData(string loopId)
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


// ###################
// CMD_GET_TL_VARIABLE
// ###################

list<string> TraCI_Extend::commandGetTLIDList()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_TL_VARIABLE, "",ID_LIST, RESPONSE_GET_TL_VARIABLE);
}

uint32_t TraCI_Extend::commandGetCurrentPhaseDuration(string TLid)
{
    return getCommandInterface()->genericGetInt(CMD_GET_TL_VARIABLE, TLid, TL_PHASE_DURATION, RESPONSE_GET_TL_VARIABLE);
}

uint32_t TraCI_Extend::commandGetCurrentPhase(string TLid)
{
    return getCommandInterface()->genericGetInt(CMD_GET_TL_VARIABLE, TLid, TL_CURRENT_PHASE, RESPONSE_GET_TL_VARIABLE);
}

string TraCI_Extend::commandGetCurrentProgram(string TLid)
{
    return getCommandInterface()->genericGetString(CMD_GET_TL_VARIABLE, TLid, TL_CURRENT_PROGRAM, RESPONSE_GET_TL_VARIABLE);
}

uint32_t TraCI_Extend::commandGetNextSwitchTime(string TLid)
{
    return getCommandInterface()->genericGetInt(CMD_GET_TL_VARIABLE, TLid, TL_NEXT_SWITCH, RESPONSE_GET_TL_VARIABLE);
}

string TraCI_Extend::commandGetTLState(string TLid)
{
    return getCommandInterface()->genericGetString(CMD_GET_TL_VARIABLE, TLid, TL_RED_YELLOW_GREEN_STATE, RESPONSE_GET_TL_VARIABLE);
}


// #####################
// CMD_GET_GUI_VARIABLE
// #####################

Coord TraCI_Extend::commandGetGUIOffset()
{
    return genericGetCoordv2(CMD_GET_GUI_VARIABLE, "View #0", VAR_VIEW_OFFSET, RESPONSE_GET_GUI_VARIABLE);
}


vector<double> TraCI_Extend::commandGetGUIBoundry()
{
    return genericGetBoundingBox(CMD_GET_GUI_VARIABLE, "View #0", VAR_VIEW_BOUNDARY, RESPONSE_GET_GUI_VARIABLE);
}


// ########################
// CMD_GET_POLYGON_VARIABLE
// ########################

list<string> TraCI_Extend::commandGetPolygonList()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_POLYGON_VARIABLE, "", ID_LIST, RESPONSE_GET_POLYGON_VARIABLE);
}


std::list<Coord> TraCI_Extend::commandGetPolygonShape(string polyId)
{
    return getCommandInterface()->genericGetCoordList(CMD_GET_POLYGON_VARIABLE, polyId, VAR_SHAPE, RESPONSE_GET_POLYGON_VARIABLE);
}



// ##############
// CMD_GET_PERSON
// ##############

uint32_t TraCI_Extend::commandGetPedestrianCount()
{
    return getCommandInterface()->genericGetInt(0xae, "", ID_COUNT, 0xbe);
}


list<string> TraCI_Extend::commandGetPedestrianList()
{
    return getCommandInterface()->genericGetStringList(0xae, "", ID_LIST, 0xbe);
}


string TraCI_Extend::commandGetPedestrianTypeId(string pId)
{
    return getCommandInterface()->genericGetString(0xae, pId, VAR_TYPE, 0xbe);
}


Coord TraCI_Extend::commandGetPedestrianPos(string pId)
{
    return genericGetCoordv2(0xae, pId, VAR_POSITION, 0xbe);
}


double TraCI_Extend::commandGetPedestrianSpeed(string pId)
{
    return getCommandInterface()->genericGetDouble(0xae, pId, VAR_SPEED, 0xbe);
}


string TraCI_Extend::commandGetPedestrianRoadId(string pId)
{
    return getCommandInterface()->genericGetString(0xae, pId, VAR_ROAD_ID, 0xbe);
}


// ###############################################
// ###############################################
// ########      SET starts from here     ########
// ###############################################
// ###############################################

// #########################
// CMD_SET_VEHICLE_VARIABLE
// #########################

// Lets the vehicle stop at the given edge, at the given position and lane.
// The vehicle will stop for the given duration.
void TraCI_Extend::commandVehicleStop(string nodeId, string edgeId, double stopPos, uint8_t laneId, double waitT, uint8_t flag)
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


void TraCI_Extend::commandChangeVehicleLane(string nodeId, uint8_t laneId, double duration)
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


void TraCI_Extend::commandChangeVehicleSpeed(string nodeId, double speed)
{
    uint8_t variableId = VAR_SPEED;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandChangeVehicleColor(std::string nodeId, const TraCIColor& color)
{
    TraCIBuffer p;
    p << static_cast<uint8_t>(VAR_COLOR);
    p << nodeId;
    p << static_cast<uint8_t>(TYPE_COLOR) << color.red << color.green << color.blue << color.alpha;
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, p);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandChangeVehicleRoute(string id, list<string> value)
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


int32_t TraCI_Extend::commandCreatLaneChangeMode(uint8_t TraciLaneChangePriority, uint8_t RightDriveLC, uint8_t SpeedGainLC, uint8_t CooperativeLC, uint8_t StrategicLC)
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


void TraCI_Extend::commandSetLaneChangeMode(string nodeId, int32_t bitset)
{
    uint8_t variableId = 0xb6;
    uint8_t variableType = TYPE_INTEGER;
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << bitset);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandAddVehicle(string vehicleId, string vehicleTypeId, string routeId, int32_t depart, double pos, double speed, uint8_t lane)
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


void TraCI_Extend::commandSetVehicleParking(string nodeId)
{
    uint8_t variableId = REMOVE;
    uint8_t variableType = TYPE_BYTE;
    uint8_t value = REMOVE_PARKING;
    TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetVehicleClass(string nodeId, string vClass)
{
    uint8_t variableId = VAR_VEHICLECLASS;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << vClass);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetVehicleMaxAccel(string nodeId, double value)
{
    uint8_t variableId = VAR_ACCEL;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetVehicleMaxDecel(string nodeId, double value)
{
    uint8_t variableId = VAR_DECEL;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


// this changes vehicle type!
void TraCI_Extend::commandSetVehicleTg(string nodeId, double value)
{
    uint8_t variableId = VAR_TAU;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetVehicleControllerParameters(string nodeId, string value)
{
    uint8_t variableId = 0x15;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetVehicleErrorGap(string nodeId, double value)
{
    uint8_t variableId = 0x20;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetVehicleErrorRelSpeed(string nodeId, double value)
{
    uint8_t variableId = 0x21;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetVehicleDegradeToACC(string nodeId, bool value)
{
    uint8_t variableId = 0x17;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << (int)value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetVehicleDebug(string nodeId, bool value)
{
    uint8_t variableId = 0x16;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << (int)value);
    ASSERT(buf.eof());
}


// ############################
// CMD_SET_VEHICLETYPE_VARIABLE
// ############################

void TraCI_Extend::commandSetMaxSpeed(string nodeId, double speed)
{
    uint8_t variableId = VAR_MAXSPEED;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetVint(string nodeId, double value)
{
    uint8_t variableId = 0x22;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetComfAccel(string nodeId, double speed)
{
    uint8_t variableId = 0x23;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetComfDecel(string nodeId, double speed)
{
    uint8_t variableId = 0x24;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLETYPE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);
    ASSERT(buf.eof());
}


// #######################
// CMD_SET_ROUTE_VARIABLE
// #######################

void TraCI_Extend::commandAddRoute(string name, list<string> route)
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


// ######################
// CMD_SET_EDGE_VARIABLE
// ######################

void TraCI_Extend::commandSetEdgeGlobalTravelTime(string edgeId, int32_t beginT, int32_t endT, double value)
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


// ######################
// CMD_SET_LANE_VARIABLE
// ######################

void TraCI_Extend::commandSetLaneVmax(string laneId, double value)
{
    uint8_t variableId = VAR_MAXSPEED;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_LANE_VARIABLE, TraCIBuffer() << variableId << laneId << variableType << value);
    ASSERT(buf.eof());
}


// ###################
// CMD_SET_TL_VARIABLE
// ###################

void TraCI_Extend::commandSetTLState(string TLid, string value)
{
    uint8_t variableId = TL_RED_YELLOW_GREEN_STATE;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetTLPhaseIndex(string TLid, int value)
{
    uint8_t variableId = TL_PHASE_INDEX;
    uint8_t variableType = TYPE_INTEGER;

    //TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_TL_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(TL_PHASE_INDEX) << TLid << static_cast<uint8_t>(TYPE_INTEGER) << value);

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetTLPhaseDurationRemaining(string TLid, int value)
{
    uint8_t variableId = TL_PHASE_DURATION;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());
}


//TODO, broken right now
void TraCI_Extend::commandSetTLPhaseDuration(string TLid, int value)
{
    uint8_t variableId = TL_PHASE_DURATION;
    uint8_t variableType = TYPE_INTEGER;

    double duration = commandGetCurrentPhaseDuration(TLid);
    cout << "Simtime: " << simTime().dbl() << endl;
    cout << "Dur: " << duration << endl;
    double remaining = commandGetNextSwitchTime(TLid) - simTime().dbl() * 1000;
    cout << "Remaining: " << remaining << endl;
    value = (value - duration + remaining);
    cout << "Value: " << value << endl;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_TL_VARIABLE, TraCIBuffer() << variableId << TLid << variableType << value);
    ASSERT(buf.eof());
}


// #####################
// CMD_SET_GUI_VARIABLE
// #####################

void TraCI_Extend::commandSetGUIZoom(double value)
{
    uint8_t variableId = VAR_VIEW_ZOOM;
    string viewID = "View #0";
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetGUIOffset(double x, double y)
{
    uint8_t variableId = VAR_VIEW_OFFSET;
    string viewID = "View #0";
    uint8_t variableType = POSITION_2D;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << x << y);
    ASSERT(buf.eof());
}


// very slow!
void TraCI_Extend::commandSetGUITrack(string nodeId)
{
    uint8_t variableId = VAR_TRACK_VEHICLE;
    string viewID = "View #0";
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << variableId << viewID << variableType << nodeId);
    ASSERT(buf.eof());
}


// ########################
// CMD_SET_POLYGON_VARIABLE
// ########################

void TraCI_Extend::commandAddPolygon(std::string polyId, std::string polyType, const TraCIColor& color, bool filled, int32_t layer, const std::list<TraCICoord>& points)
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


// ############################
// generic methods for getters
// ############################

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


}

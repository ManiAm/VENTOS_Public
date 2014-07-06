
#include "Global_01_TraCI_Extend.h"
#include <sstream>
#include <fstream>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>

struct traci2omnet_functor : public std::unary_function<TraCICoord, Coord> {
    traci2omnet_functor(const TraCIScenarioManager& m) : manager(m) {}

    Coord operator()(const TraCICoord& coord) const {
        return manager.traci2omnet(coord);
    }

    const TraCIScenarioManager& manager;
};


Define_Module(TraCI_Extend);


TraCI_Extend::~TraCI_Extend()
{

}


void TraCI_Extend::initialize(int stage)
{
    TraCIScenarioManager::initialize(stage);

    if (stage == 1)
    {
        seed = par("seed");

        boost::filesystem::path SUMODirectory = simulation.getSystemModule()->par("SUMODirectory").stringValue();
        boost::filesystem::path VENTOSfullDirectory = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        SUMOfullDirectory = VENTOSfullDirectory / SUMODirectory;   //home/mani/Desktop/VENTOS/sumo/CACC_Platoon

        // check if this directory is valid?
        if( !exists( SUMOfullDirectory ) )
        {
            error("SUMO directory is not valid! Check it again.");
        }
    }
    else if(stage == 2)
    {
        // todo: here we can put the equivalent python
        // code to create the server process
    }
}


void TraCI_Extend::init_traci()
{
    {
    // get the version of Python launchd
    pair<uint32_t, string> version = getCommandInterface()->getVersion();
    uint32_t apiVersion = version.first;
    string serverVersion = version.second;
    EV << "TraCI launchd reports apiVersion: " << apiVersion << " and serverVersion: " << serverVersion << endl;
    ASSERT(apiVersion == 1);
    }

    // ---------------------------------------------------

    {
    string str = createLaunch();
    commandSendFile( str );   // send the launchConfig to SUMO
    }

    // ---------------------------------------------------

    {
    // call commandGetVersion again to get the version of SUMO TraCI server
    pair<uint32_t, string> version2 = getCommandInterface()->getVersion();
    uint32_t apiVersion2 = version2.first;
    string serverVersion2 = version2.second;
    EV << "TraCI server reports apiVersion: " << apiVersion2 << " and serverVersion: " << serverVersion2 << endl;
    if ( !(apiVersion2 == 3 || apiVersion2 == 5 || apiVersion2 == 6 || apiVersion2 == 7 || apiVersion2 == 8) )
        error("TraCI server is unsupported.");
    }

    // ---------------------------------------------------

    {
    // get network boundaries from SUMO
    double* bounds = commandGetNetworkBoundary();
    EV << "TraCI reports network boundaries (" << bounds[0] << ", " << bounds[1] << ")-(" << bounds[2] << ", " << bounds[3] << ")" << endl;

    // change to OMNET++ coordinates
    netbounds1 = TraCICoord(bounds[0], bounds[1]);
    netbounds2 = TraCICoord(bounds[2], bounds[3]);
    if ((traci2omnet(netbounds2).x > world->getPgs()->x) || (traci2omnet(netbounds1).y > world->getPgs()->y))
        EV << "WARNING: Playground size (" << world->getPgs()->x << ", " << world->getPgs()->y << ") might be too small for vehicle at network bounds (" << traci2omnet(netbounds2).x << ", " << traci2omnet(netbounds1).y << ")" << endl;
    }
    // ---------------------------------------------------

    {
    // subscribe to list of departed and arrived vehicles, as well as simulation time
    commandSubscribeSimulation();
    }

    {
    // subscribe to list of vehicle ids
    commandSubscribeVehicle();
    }

    ObstacleControl* obstacles = ObstacleControlAccess().getIfExists();
    if (obstacles)
    {
        {
            // get list of polygons
            std::list<std::string> ids = getCommandInterface()->getPolygonIds();
            for (std::list<std::string>::iterator i = ids.begin(); i != ids.end(); ++i)
            {
                std::string id = *i;
                std::string typeId = getCommandInterface()->getPolygonTypeId(id);

                if (typeId == "building")
                {
                    std::list<TraCICoord> coords = getCommandInterface()->getPolygonShape(id);
                    Obstacle obs(id, 9, .4); // each building gets attenuation of 9 dB per wall, 0.4 dB per meter
                    std::vector<Coord> shape;
                    std::transform(coords.begin(), coords.end(), std::back_inserter(shape), traci2omnet_functor(*this));
                    obs.setShape(shape);
                    obstacles->add(obs);
                }
            }
        }
    }
}


string TraCI_Extend::createLaunch()
{
    string launchFile = par("launchFile").stringValue();
    boost::filesystem::path launchFullPath = SUMOfullDirectory / launchFile;

    file<> xmlFile( launchFullPath.string().c_str() );
    xml_document<> doc;
    doc.parse<0>(xmlFile.data());

    // create a node called basedir
    xml_node<> *basedir = doc.allocate_node(node_element, "basedir");

    // append basedir to the launch
    xml_node<> *node = doc.first_node("launch");
    node->append_node(basedir);

    // append attribute to basedir
    xml_attribute<> *attr = doc.allocate_attribute("path", SUMOfullDirectory.c_str());
    basedir->append_attribute(attr);

    if (seed == -1)
    {
        // default seed is current repetition
        const char* seed_s = cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getVariable(CFGVAR_RUNNUMBER);
        seed = atoi(seed_s);
    }

    // create a node called seed
    xml_node<> *seedN = doc.allocate_node(node_element, "seed");

    // append seed to the launch
    xml_node<> *node1 = doc.first_node("launch");
    node1->append_node(seedN);

    // append attribute to seed
    stringstream ss; ss << seed;
    xml_attribute<> *attr1 = doc.allocate_attribute( "value", ss.str().c_str() );
    seedN->append_attribute(attr1);

    // todo:

    // write the new file
    string newFileName = "updated_" + launchFile;
    boost::filesystem::path newLaunchFullPath = SUMOfullDirectory / newFileName;
    ofstream file_stored( newLaunchFullPath.string().c_str() );
    file_stored << doc;
    file_stored.close();

    ifstream t(newLaunchFullPath.string().c_str());
    stringstream buffer;
    buffer << t.rdbuf();

    return buffer.str();
}


void TraCI_Extend::handleSelfMsg(cMessage *msg)
{
    TraCIScenarioManager::handleSelfMsg(msg);
}


// #########################
// CMD_GET_VEHICLE_VARIABLE
// #########################

uint32_t TraCI_Extend::commandGetNoVehicles()
{
    return getCommandInterface()->genericGetInt(CMD_GET_VEHICLE_VARIABLE, "", ID_COUNT, RESPONSE_GET_VEHICLE_VARIABLE);
}


// gets a list of all vehicles in the network (alphabetically!!!)
list<string> TraCI_Extend::commandGetVehicleList()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_VEHICLE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::commandGetVehicleSpeed(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_SPEED, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::commandGetVehicleAccel(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, 0x41, RESPONSE_GET_VEHICLE_VARIABLE);
}


string TraCI_Extend::commandGetVehicleType(string nodeId)
{
    return getCommandInterface()->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TYPE, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::commandGetVehicleLength(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LENGTH, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::commandGetVehicleMinGap(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_MINGAP, RESPONSE_GET_VEHICLE_VARIABLE);
}


double TraCI_Extend::commandGetVehicleMaxDecel(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_DECEL, RESPONSE_GET_VEHICLE_VARIABLE);
}


Coord TraCI_Extend::commandGetVehiclePos(string nodeId)
{
    return genericGetCoordv2(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_POSITION, RESPONSE_GET_VEHICLE_VARIABLE);
}


uint32_t TraCI_Extend::commandGetLaneIndex(string nodeId)
{
    return getCommandInterface()->genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_INDEX, RESPONSE_GET_VEHICLE_VARIABLE);
}


vector<string> TraCI_Extend::commandGetLeading(string nodeId, double look_ahead_distance)
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


// my own method! [deprecated!]
string TraCI_Extend::commandGetLeading_old(string nodeId)
{
    // get the lane id (like 1to2_0)
    string laneID = getCommandInterface()->getLaneId(nodeId);

    // get a list of all vehicles on this lane (left to right)
    list<string> myList = commandGetVehiclesOnLane(laneID);

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


uint8_t* TraCI_Extend::commandGetVehicleColor(string nodeId)
{
    return genericGetArrayUnsignedInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_COLOR, RESPONSE_GET_VEHICLE_VARIABLE);
}


// ############################
// CMD_GET_VEHICLETYPE_VARIABLE
// ############################

double TraCI_Extend::commandGetVehicleLength_Type(string nodeId)
{
    return getCommandInterface()->genericGetDouble(CMD_GET_VEHICLETYPE_VARIABLE, nodeId, VAR_LENGTH, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}


// ######################
// CMD_GET_ROUTE_VARIABLE
// ######################

list<string> TraCI_Extend::commandGetRouteIds()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_ROUTE_VARIABLE, "", 0x00, RESPONSE_GET_ROUTE_VARIABLE);
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


// #####################
// CMD_GET_LANE_VARIABLE
// #####################

// gets a list of all lanes in the network
list<string> TraCI_Extend::commandGetLaneList()
{
    return getCommandInterface()->genericGetStringList(CMD_GET_LANE_VARIABLE, "", ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}


// gets a list of all lanes in the network
list<string> TraCI_Extend::commandGetVehicleLaneList(string edgeId)
{
    return getCommandInterface()->genericGetStringList(CMD_GET_LANE_VARIABLE, edgeId, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}


list<string> TraCI_Extend::commandGetVehiclesOnLane(string laneId)
{
    return getCommandInterface()->genericGetStringList(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_VEHICLE_ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}


// #####################
// CMD_SET_GUI_VARIABLE
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
// RSU
// ########################

deque<RSUEntry*> TraCI_Extend::commandReadRSUsCoord(string RSUfilePath)
{
    file<> xmlFile( RSUfilePath.c_str() );        // Convert our file to a rapid-xml readable object
    xml_document<> doc;                           // Build a rapidxml doc
    doc.parse<0>(xmlFile.data());                 // Fill it with data from our file
    xml_node<> *node = doc.first_node("RSUs");    // Parse up to the "RSUs" declaration

    for(node = node->first_node("poly"); node; node = node->next_sibling())
    {
        string RSUname = "";
        string RSUtype = "";
        string RSUcoordinates = "";
        int readCount = 1;

        // For each node, iterate over its attributes until we reach "shape"
        for(xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
        {
            if(readCount == 1)
            {
                RSUname = attr->value();
            }
            else if(readCount == 2)
            {
                RSUtype = attr->value();
            }
            else if(readCount == 3)
            {
                RSUcoordinates = attr->value();
            }

            readCount++;
        }

        // tokenize coordinates
        int readCount2 = 1;
        double x;
        double y;
        char_separator<char> sep(",");
        tokenizer< char_separator<char> > tokens(RSUcoordinates, sep);

        for(tokenizer< char_separator<char> >::iterator beg=tokens.begin(); beg!=tokens.end();++beg)
        {
            if(readCount2 == 1)
            {
                x = atof( (*beg).c_str() );
            }
            else if(readCount2 == 2)
            {
                y = atof( (*beg).c_str() );
            }

            readCount2++;
        }

        // add it into queue (with TraCI coordinates)
        RSUEntry *entry = new RSUEntry(RSUname.c_str(), x, y);
        RSUs.push_back(entry);
    }

    return RSUs;
}


Coord *TraCI_Extend::commandGetRSUsCoord(unsigned int index)
{
    if( RSUs.size() == 0 )
        error("No RSUs have been initialized!");

    if( index < 0 || index >= RSUs.size() )
        error("index out of bound!");

    Coord *point = new Coord(RSUs[index]->coordX, RSUs[index]->coordY);
    return point;
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


// #########################
// Control-related commands
// #########################

void TraCI_Extend::commandTerminate()
{
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_CLOSE, TraCIBuffer() << 0);

    uint32_t count;
    buf >> count;
}


void TraCI_Extend::commandSendFile(string contents)
{
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


// ######################
// subscription commands
// ######################

void TraCI_Extend::commandSubscribeSimulation()
{
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

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SUBSCRIBE_SIM_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1 << variable2 << variable3 << variable4 << variable5 << variable6 << variable7);

    processSubcriptionResult(buf);

    ASSERT(buf.eof());
}


void TraCI_Extend::commandSubscribeVehicle()
{
    uint32_t beginTime = 0;
    uint32_t endTime = 0x7FFFFFFF;
    string objectId = "";
    uint8_t variableNumber = 1;
    uint8_t variable1 = ID_LIST;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SUBSCRIBE_VEHICLE_VARIABLE, TraCIBuffer() << beginTime << endTime
                                                                                            << objectId
                                                                                            << variableNumber
                                                                                            << variable1
                                                                                            );
    processSubcriptionResult(buf);

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

// #########################
// CMD_SET_ROUTE_VARIABLE
// #########################

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


// #########################
// CMD_SET_VEHICLE_VARIABLE
// #########################

void TraCI_Extend::commandSetRouteFromList(string id, list<string> value)
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


void TraCI_Extend::commandSetSpeed(string nodeId, double speed)
{
    uint8_t variableId = VAR_SPEED;
    uint8_t variableType = TYPE_DOUBLE;
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetMaxAccel(string nodeId, double value)
{
    uint8_t variableId = VAR_ACCEL;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetMaxDecel(string nodeId, double value)
{
    uint8_t variableId = VAR_DECEL;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


// this changes vehicle type!
void TraCI_Extend::commandSetTg(string nodeId, double value)
{
    uint8_t variableId = VAR_TAU;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetLaneChangeMode(string nodeId, int32_t bitset)
{
    uint8_t variableId = 0xb6;
    uint8_t variableType = TYPE_INTEGER;
    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << bitset);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandAddVehicleN(string vehicleId, string vehicleTypeId, string routeId, int32_t depart, double pos, double speed, uint8_t lane)
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


void TraCI_Extend::commandSetCFParameters(string nodeId, string value)
{
    uint8_t variableId = 0x15;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetDebug(string nodeId, bool value)
{
    uint8_t variableId = 0x16;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << (int)value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetModeSwitch(string nodeId, bool value)
{
    uint8_t variableId = 0x17;
    uint8_t variableType = TYPE_INTEGER;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << (int)value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandRemoveVehicle(string nodeId, uint8_t reason)
{
    uint8_t variableId = REMOVE;
    uint8_t variableType = TYPE_BYTE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << reason);
    ASSERT(buf.eof());

    activeVehicleCount--;
    TraCIScenarioManager::deleteModule(nodeId);
}


void TraCI_Extend::commandStopNodeExtended(string nodeId, string edgeId, double stopPos, uint8_t laneId, double waitT, uint8_t flag)
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


void TraCI_Extend::commandSetvClass(string nodeId, string vClass)
{
    uint8_t variableId = VAR_VEHICLECLASS;
    uint8_t variableType = TYPE_STRING;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << vClass);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandChangeLane(string nodeId, uint8_t laneId, double duration)
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


void TraCI_Extend::commandSetErrorGap(string nodeId, double value)
{
    uint8_t variableId = 0x20;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
    ASSERT(buf.eof());
}


void TraCI_Extend::commandSetErrorRelSpeed(string nodeId, double value)
{
    uint8_t variableId = 0x21;
    uint8_t variableType = TYPE_DOUBLE;

    TraCIBuffer buf = getCommandInterface()->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
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


// #####################
// Polygon
// #####################

void TraCI_Extend::commandAddCirclePoly(string name, string type, const TraCIColor& color, Coord *center, double radius)
{
    list<TraCICoord> circlePoints;

    // Convert from degrees to radians via multiplication by PI/180
    for(int angleInDegrees = 0; angleInDegrees <= 360; angleInDegrees = angleInDegrees + 10)
    {
        double x = (double)( radius * cos(angleInDegrees * 3.14 / 180) ) + center->x;
        double y = (double)( radius * sin(angleInDegrees * 3.14 / 180) ) + center->y;

        circlePoints.push_back(TraCICoord(x, y));
    }

    // create polygon in SUMO
    getCommandInterface()->addPolygon(name, type, color, 0, 1, circlePoints);
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




void TraCI_Extend::finish()
{

}


/****************************************************************************/
/// @file    AddNode.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Jan 2017
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

#undef ev
#include "boost/filesystem.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <random>

#include "addNode/AddNode.h"
#include "MIXIM_veins/connectionManager/ConnectionManager.h"
#include "logging/VENTOS_logging.h"
#include "xmlUtil.h"

namespace VENTOS {

Define_Module(VENTOS::AddNode);

AddNode::~AddNode()
{

}


void AddNode::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        // check module names for duplicate -- even if addNode id is empty
        checkDuplicateModuleName(par("vehicle_ModuleName").stringValue());
        checkDuplicateModuleName(par("bike_ModuleName").stringValue());
        checkDuplicateModuleName(par("ped_ModuleName").stringValue());
        checkDuplicateModuleName(par("obstacle_ModuleName").stringValue());
        checkDuplicateModuleName(par("RSU_ModuleName").stringValue());
        checkDuplicateModuleName(par("adversary_ModuleName").stringValue());
        checkDuplicateModuleName(par("CA_ModuleName").stringValue());

        id = par("id").stringValue();
        if(id == "")
            return;

        // get a pointer to the TraCI module
        TraCI = TraCI_Commands::getTraCI();

        terminateTime = TraCI->par("terminateTime").doubleValue();

        Signal_initialize_withTraCI = registerSignal("initializeWithTraCISignal");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initializeWithTraCISignal", this);

        Signal_executeEachTS = registerSignal("executeEachTimeStepSignal");
        omnetpp::getSimulation()->getSystemModule()->subscribe("executeEachTimeStepSignal", this);
    }
}


void AddNode::finish()
{
    omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("connMan");
    ConnectionManager *cc = static_cast<ConnectionManager*>(module);
    ASSERT(cc);

    // delete all adversary modules in omnet
    for(auto &i : allAdversary)
    {
        cModule* mod = i.second.module;
        ASSERT(mod);

        cc->unregisterNic(mod->getSubmodule("nic"));
        mod->callFinish();
        mod->deleteModule();

        TraCI->removeMapping(i.second.id_str);
    }

    // delete all RSU modules in omnet
    for(auto &i : allRSU)
    {
        cModule* mod = i.second.module;
        ASSERT(mod);

        cc->unregisterNic(mod->getSubmodule("nic"));
        mod->callFinish();
        mod->deleteModule();

        TraCI->removeMapping(i.second.id_str);
    }

    // delete all CA modules in omnet
    for(auto &i : allCA)
    {
        cModule* mod = i.second.module;
        ASSERT(mod);

        cc->unregisterNic(mod->getSubmodule("nic"));
        mod->callFinish();
        mod->deleteModule();

        TraCI->removeMapping(i.second.id_str);
    }

    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("initializeWithTraCISignal", this);
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("executeEachTimeStepSignal", this);
}


void AddNode::handleMessage(omnetpp::cMessage *msg)
{
    if(msg->isSelfMessage() && msg->getKind() == TYPE_TIMER_OBSTACLE)
    {
        std::string vehID = msg->getName();

        // we ask SUMO to remove the obstacle from SUMO.
        // TraCIStart class will automatically remove the cModule
        TraCI->vehicleRemove(vehID, 2);
    }
    else if(msg->isSelfMessage() && msg->getKind() == TYPE_TIMER_STOPPED_VEHICLE)
    {
        std::string vehID = msg->getName();

        auto it = allVehicle.find(vehID);
        if(it == allVehicle.end())
            throw omnetpp::cRuntimeError("Cannot find veh '%s' in the map", vehID.c_str());

        // set the lane change mode
        TraCI->vehicleSetLaneChangeMode(vehID, it->second.laneChangeMode);

        // get the max speed
        double vMax = TraCI->vehicleGetMaxSpeed(vehID);

        TraCI->vehicleSetSpeed(vehID, vMax);
    }
    else
        throw omnetpp::cRuntimeError("Cannot handle msg '%s' of type '%d'", msg->getFullName(), msg->getKind());
}


void AddNode::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        SUMO_timeStep = (double)TraCI->simulationGetDelta() / 1000.;
        if(SUMO_timeStep <= 0)
            throw omnetpp::cRuntimeError("Simulation time step '%d' is invalid", SUMO_timeStep);

        readXMLFile("addNode.xml");
    }
    else if(signalID == Signal_executeEachTS)
    {
        // platoons are added in run-time
        if(!allVehiclePlatoon.empty())
            addVehiclePlatoon();
    }
}


void AddNode::checkDuplicateModuleName(std::string moduleName)
{
    static std::vector<std::string> allModuleNames;

    auto ii = std::find(allModuleNames.begin(), allModuleNames.end(), moduleName);
    if(ii == allModuleNames.end())
        allModuleNames.push_back(moduleName);
    else
        throw omnetpp::cRuntimeError("Two or more module names in AddNode have the same id '%s'", moduleName.c_str());
}


void AddNode::readXMLFile(std::string addNodePath)
{
    rapidxml::file<> xmlFile(addNodePath.c_str());  // Convert our file to a rapid-xml readable object
    rapidxml::xml_document<> doc;                   // Build a rapidxml doc
    doc.parse<0>(xmlFile.data());                   // Fill it with data from our file

    // Get the first applDependency node
    rapidxml::xml_node<> *pNode = doc.first_node("addNode");

    if(pNode == NULL)
    {
        LOG_WARNING << boost::format("\nWARNING: There is no 'addNode' nodes in the addNode.xml file \n") << std::flush;
        return;
    }

    while(1)
    {
        // Get id attribute
        rapidxml::xml_attribute<> *pAttr = pNode->first_attribute("id");

        // Get the value of this attribute
        std::string strValue = pAttr->value();

        // We found the correct applDependency node
        if(strValue == this->id)
            break;
        // Get the next applDependency
        else
        {
            pNode = pNode->next_sibling();
            if(!pNode)
                throw omnetpp::cRuntimeError("Cannot find id '%s' in the addNode.xml file!", this->id.c_str());
        }
    }

    // iterate over all nodes in this id
    for(rapidxml::xml_node<> *cNode = pNode->first_node(); cNode; cNode = cNode->next_sibling())
        parsXMLFile(cNode);

    if(allAdversary.empty() &&
            allRSU.empty() &&
            allObstacle.empty() &&
            allVehicle.empty() &&
            allVehicleFlow.empty() &&
            allVehicleMultiFlow.empty() &&
            allVehiclePlatoon.empty() &&
            allCA.empty())
        LOG_WARNING << boost::format("\nWARNING: Add node with id '%1%' is empty! \n") % this->id << std::flush;

    insertNodes();

    if(!allVehicle.empty() || !allVehicleFlow.empty() || !allVehicleMultiFlow.empty() || !allVehiclePlatoon.empty())
    {
        if(LOG_ACTIVE(DEBUG_LOG_VAL))
            printLoadedStatistics();
    }
}


void AddNode::parsXMLFile(rapidxml::xml_node<> *cNode)
{
    std::string nodeName = cNode->name();

    if(nodeName == adversary_tag)
        parseAdversary(cNode);
    else if(nodeName == rsu_tag)
        parseRSU(cNode);
    else if(nodeName == obstacle_tag)
        parseObstacle(cNode);
    else if(nodeName == vehicle_tag)
        parseVehicle(cNode);
    else if(nodeName == vehicle_flow_tag)
        parseVehicleFlow(cNode);
    else if(nodeName == vehicle_multiFlow_tag)
        parseVehicleMultiFlow(cNode);
    else if(nodeName == vehicle_platoon_tag)
        parseVehiclePlatoon(cNode);
    else if(nodeName == ca_tag)
        parseCA(cNode);
    else
        throw omnetpp::cRuntimeError("'%s' is not a valid element in id '%s' of addNode.xml file!", nodeName.c_str(), this->id.c_str());
}


void AddNode::insertNodes()
{
    addAdversary();
    addRSU();
    addObstacle();
    addVehicle();
    addVehicleFlow();
    addVehicleMultiFlow();
    addVehiclePlatoon();  // our first attempt in adding platoons
    addCA();
}


void AddNode::parseAdversary(rapidxml::xml_node<> *cNode)
{
    ASSERT(std::string(cNode->name()) == adversary_tag);

    std::vector<std::string> validAttr = {"id", "pos", "drawMaxIntfDist"};
    xmlUtil::validityCheck(cNode, validAttr);

    std::string id_str = xmlUtil::getAttrValue_string(cNode, "id");
    TraCICoord pos = xmlUtil::getAttrValue_coord(cNode, "pos");
    bool drawMaxIntfDist = xmlUtil::getAttrValue_bool(cNode, "drawMaxIntfDist", false, true);
    std::string color_str = xmlUtil::getAttrValue_string(cNode, "color", false, "green");
    bool filled = xmlUtil::getAttrValue_bool(cNode, "filled", false, false);

    auto it = allAdversary.find(id_str);
    if(it == allAdversary.end())
    {
        // check if the new node has overlap with any of the existing nodes
        for(auto &entry : allAdversary)
        {
            if(entry.second.pos == pos)
                LOG_WARNING << boost::format("WARNING: Adversary '%s' is placed on top of '%s'. \n") % id_str % entry.second.id_str;
        }

        adversaryEntry_t entry = {};

        entry.id_str = id_str;
        entry.pos = pos;
        entry.drawMaxIntfDist = drawMaxIntfDist;
        entry.color_str = color_str;
        entry.filled = filled;

        allAdversary.insert(std::make_pair(id_str, entry));
    }
    else
        throw omnetpp::cRuntimeError("Multiple %s with the same 'id' %s is not allowed!", adversary_tag.c_str(), id_str.c_str());
}


void AddNode::addAdversary()
{
    if(allAdversary.empty())
        return;

    unsigned int num = allAdversary.size();

    LOG_DEBUG << boost::format("\n>>> AddNode is adding %1% adversary modules ... \n") % num << std::flush;

    cModule* parentMod = getParentModule();
    if (!parentMod)
        throw omnetpp::cRuntimeError("Parent Module not found");

    omnetpp::cModuleType* nodeType = omnetpp::cModuleType::get(par("adversary_ModuleType"));

    int i = 0;
    for(auto &entry : allAdversary)
    {
        // create an array of adversaries
        cModule* mod = nodeType->create(par("adversary_ModuleName"), parentMod, num, i);
        mod->finalizeParameters();
        mod->getDisplayString().parse(par("adversary_ModuleDisplayString"));
        mod->buildInside();

        TraCI->addMapping(entry.second.id_str, mod->getFullName());

        Coord co = TraCI->convertCoord_traci2omnet(entry.second.pos);

        mod->getSubmodule("mobility")->par("x") = co.x;
        mod->getSubmodule("mobility")->par("y") = co.y;
        mod->getSubmodule("mobility")->par("z") = co.z;

        mod->getSubmodule("nic")->par("drawMaxIntfDist") = entry.second.drawMaxIntfDist;

        mod->scheduleStart(omnetpp::simTime());
        mod->callInitialize();

        // store the cModule
        entry.second.module = mod;

        i++;
    }

    // now we draw adversary modules in SUMO (using a circle to show radio coverage)
    i = 0;
    for(auto &entry : allAdversary)
    {
        // get a reference to this adversary
        omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule(par("adversary_ModuleName"), i);
        ASSERT(module);

        // get the radius
        double radius = atof( module->getDisplayString().getTagArg("r",0) );

        if(entry.second.drawMaxIntfDist && radius > 0)
            addCircle(entry.second.id_str, par("adversary_ModuleName"), Color::colorNameToRGB(entry.second.color_str), entry.second.filled, entry.second.pos, radius);

        i++;
    }
}


void AddNode::parseRSU(rapidxml::xml_node<> *cNode)
{
    ASSERT(std::string(cNode->name()) == rsu_tag);

    std::vector<std::string> validAttr = {"id", "pos", "drawMaxIntfDist", "color", "filled"};
    xmlUtil::validityCheck(cNode, validAttr);

    std::string id_str = xmlUtil::getAttrValue_string(cNode, "id");
    TraCICoord pos = xmlUtil::getAttrValue_coord_rnd(cNode, "pos");
    bool drawMaxIntfDist = xmlUtil::getAttrValue_bool(cNode, "drawMaxIntfDist", false, true);
    std::string color_str = xmlUtil::getAttrValue_string(cNode, "color", false, "green");
    bool filled = xmlUtil::getAttrValue_bool(cNode, "filled", false, false);

    auto it = allRSU.find(id_str);
    if(it == allRSU.end())
    {
        // check if the new node has overlap with any of the existing nodes
        for(auto &entry : allRSU)
        {
            if(entry.second.pos == pos)
                LOG_WARNING << boost::format("WARNING: RSU '%s' is placed on top of '%s'. \n") % id_str % entry.second.id_str;
        }

        RSUEntry_t entry = {};

        entry.id_str = id_str;
        entry.pos = pos;
        entry.drawMaxIntfDist = drawMaxIntfDist;
        entry.color_str = color_str;
        entry.filled = filled;

        allRSU.insert(std::make_pair(id_str, entry));
    }
    else
        throw omnetpp::cRuntimeError("Multiple '%s' with the same 'id' %s is not allowed!", rsu_tag.c_str(), id_str.c_str());
}


void AddNode::addRSU()
{
    if(allRSU.empty())
        return;

    unsigned int num = allRSU.size();

    LOG_DEBUG << boost::format("\n>>> AddNode is adding %1% RSU modules ... \n") % num << std::flush;

    cModule* parentMod = getParentModule();
    if (!parentMod)
        throw omnetpp::cRuntimeError("Parent Module not found");

    omnetpp::cModuleType* nodeType = omnetpp::cModuleType::get(par("RSU_ModuleType"));

    // get all traffic lights in the network
    auto TLList = TraCI->TLGetIDList();

    int i = 0;
    for(auto &entry : allRSU)
    {
        // check if any TLid is associated with this RSU
        std::string myTLid = "";
        for(std::string TLid : TLList)
        {
            if(TLid == entry.second.id_str)
            {
                myTLid = TLid;
                break;
            }
        }

        // create an array of RSUs
        cModule* mod = nodeType->create(par("RSU_ModuleName"), parentMod, num, i);
        mod->finalizeParameters();
        mod->getDisplayString().parse(par("RSU_ModuleDisplayString"));
        mod->buildInside();

        TraCI->addMapping(entry.second.id_str, mod->getFullName());

        Coord co = TraCI->convertCoord_traci2omnet(entry.second.pos);

        mod->getSubmodule("mobility")->par("x") = co.x;
        mod->getSubmodule("mobility")->par("y") = co.y;
        mod->getSubmodule("mobility")->par("z") = co.z;

        mod->par("myTLid") = myTLid;
        mod->par("SUMOID") = entry.second.id_str;

        mod->getSubmodule("nic")->par("drawMaxIntfDist") = entry.second.drawMaxIntfDist;

        mod->scheduleStart(omnetpp::simTime());
        mod->callInitialize();

        // store the cModule of this RSU
        entry.second.module = mod;

        i++;
    }

    // now we draw RSUs in SUMO (using a circle to show radio coverage)
    i = 0;
    for(auto &entry : allRSU)
    {
        // get a reference to this RSU
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule(par("RSU_ModuleName"), i);
        ASSERT(module);

        // get the radius of this RSU
        double radius = atof( module->getDisplayString().getTagArg("r",0) );

        if(entry.second.drawMaxIntfDist && radius > 0)
            addCircle(entry.second.id_str, par("RSU_ModuleName"), Color::colorNameToRGB(entry.second.color_str), entry.second.filled, entry.second.pos, radius);

        i++;
    }
}


void AddNode::parseObstacle(rapidxml::xml_node<> *cNode)
{
    ASSERT(std::string(cNode->name()) == obstacle_tag);

    std::vector<std::string> validAttr = {"id", "length", "edge", "lane", "lanePos", "onRoad", "color", "begin", "end", "duration"};
    xmlUtil::validityCheck(cNode, validAttr);

    std::string id_str = xmlUtil::getAttrValue_string(cNode, "id");
    int length = xmlUtil::getAttrValue_int(cNode, "length", false, 5);
    std::string edge_str = xmlUtil::getAttrValue_string(cNode, "edge");
    int lane = xmlUtil::getAttrValue_int(cNode, "lane", false, -5 /*DEPART_LANE_BEST_FREE*/);
    double lanePos = xmlUtil::getAttrValue_double(cNode, "lanePos");
    bool onRoad = xmlUtil::getAttrValue_bool(cNode, "onRoad", false, true);
    std::string color_str = xmlUtil::getAttrValue_string(cNode, "color", false, "red");
    double begin = xmlUtil::getAttrValue_double(cNode, "begin", false, 0);
    double end = xmlUtil::getAttrValue_double(cNode, "end", false, -1);
    double duration = xmlUtil::getAttrValue_double(cNode, "duration", false, -1);

    if(!onRoad && cNode->first_attribute("lane"))
        throw omnetpp::cRuntimeError("attribute 'lane' is redundant when 'onRoad' is false in element '%s'", obstacle_tag.c_str());

    if(cNode->first_attribute("end") && cNode->first_attribute("duration"))
        throw omnetpp::cRuntimeError("attribute 'duration' and 'end' cannot be present together in element '%s'", obstacle_tag.c_str());

    auto it = allObstacle.find(id_str);
    if(it == allObstacle.end())
    {
        // check if the new node has overlap with any of the existing nodes
        for(auto &entry : allObstacle)
        {
            if(entry.second.edge_str == edge_str && entry.second.lane == lane && entry.second.lanePos == lanePos)
                LOG_WARNING << boost::format("WARNING: Obstacle '%s' is placed on top of '%s'. \n") % id_str % entry.second.id_str;
        }

        obstacleEntry_t entry = {};

        entry.id_str = id_str;
        entry.length = length;
        entry.edge_str = edge_str;
        entry.lane = lane;
        entry.lanePos = lanePos;
        entry.onRoad = onRoad;
        entry.color_str = color_str;
        entry.begin = begin;
        entry.end = end;
        entry.duration = duration;

        allObstacle.insert(std::make_pair(id_str, entry));
    }
    else
        throw omnetpp::cRuntimeError("Multiple '%s' with the same 'id' %s is not allowed!", obstacle_tag.c_str(), id_str.c_str());
}


void AddNode::addObstacle()
{
    if(allObstacle.empty())
        return;

    unsigned int num = allObstacle.size();

    LOG_DEBUG << boost::format("\n>>> AddNode is adding %1% obstacle modules ... \n") % num << std::flush;

    auto allRouteIDs = TraCI->routeGetIDList();

    for(auto &entry : allObstacle)
    {
        std::string vehID = entry.second.id_str;

        // create a new route that consists of 'edge_str'
        std::vector<std::string> newRoute = {entry.second.edge_str};
        std::string route_name = "OBS_" + entry.second.edge_str + "_route";

        // if not added into route list
        if(std::find(allRouteIDs.begin(), allRouteIDs.end(), route_name) == allRouteIDs.end())
        {
            TraCI->routeAdd(route_name, newRoute);
            allRouteIDs.push_back(route_name);
        }

        if(entry.second.onRoad)
        {
            // now we add a vehicle as obstacle
            TraCI->vehicleAdd(vehID, "DEFAULT_VEHTYPE", route_name, (int32_t)(entry.second.begin * 1000), entry.second.lanePos, 0, entry.second.lane);

            // and make it stop on the lane!
            TraCI->vehicleSetSpeed(vehID, 0.);

            TraCI->vehicleSetLaneChangeMode(vehID, LANECHANGEMODE_OBSTACLE);
        }
        else
        {
            // get the lanes that allows vehicles class 'custome1'
            auto allowedLanes = TraCI->edgeGetAllowedLanes(entry.second.edge_str, "custom1");

            // choose either of the allowed lanes and split the laneId
            std::vector<std::string> tok;
            boost::split(tok, allowedLanes[0], boost::is_any_of("_"));

            int laneIndex = std::atoi(tok.back().c_str());

            // now we add a vehicle as obstacle
            TraCI->vehicleAdd(vehID, "DEFAULT_VEHTYPE", route_name, (int32_t)(entry.second.begin * 1000), entry.second.lanePos, 0, laneIndex);

            // and make it stop on the lane!
            TraCI->vehicleSetSpeed(vehID, 0.);

            TraCI->vehicleSetStop(vehID, entry.second.edge_str, entry.second.lanePos, laneIndex, std::numeric_limits<int32_t>::max() /*duration*/, 1 /*parking*/);
        }

        // and change its color
        RGB newColor = Color::colorNameToRGB(entry.second.color_str);
        TraCI->vehicleSetColor(vehID, newColor);

        // change veh class to "custome1"
        TraCI->vehicleSetClass(vehID, "custom1");

        // turn off all signals
        TraCI->vehicleSetSignalStatus(vehID, 0);

        // change veh length
        TraCI->vehicleSetLength(vehID, entry.second.length);

        // change veh shape
        // todo: update to new version of sumo and set shape

        double begin = entry.second.begin;
        double end = entry.second.end;
        double duration = entry.second.duration;

        if(end != -1)
        {
            if(end <= begin)
                throw omnetpp::cRuntimeError("'end' value (%f) is less than 'begin' value (%f) in element '%s'", end, begin, obstacle_tag.c_str());

            omnetpp::cMessage* evt = new omnetpp::cMessage(vehID.c_str(), TYPE_TIMER_OBSTACLE);
            scheduleAt(omnetpp::simTime() + 2*SUMO_timeStep + end, evt);
        }
        else if(duration != -1)
        {
            if(duration <= 0)
                throw omnetpp::cRuntimeError("'duration' value (%f) should be positive in element '%s'", duration, obstacle_tag.c_str());

            if(terminateTime != -1 && duration > terminateTime)
                throw omnetpp::cRuntimeError("'duration' value (%f) cannot be bigger than the simulation time (%f) in element '%s'", duration, terminateTime, obstacle_tag.c_str());

            omnetpp::cMessage* evt = new omnetpp::cMessage(vehID.c_str(), TYPE_TIMER_OBSTACLE);
            scheduleAt(omnetpp::simTime() + 2*SUMO_timeStep + begin + duration, evt);
        }
    }
}


void AddNode::parseVehicle(rapidxml::xml_node<> *cNode)
{
    ASSERT(std::string(cNode->name()) == vehicle_tag);

    std::vector<std::string> validAttr = {"id", "type", "route", "from", "to", "via", "color",
            "depart", "departLane", "departPos", "departSpeed", "laneChangeMode", "status", "duration", "DSRCprob"};
    xmlUtil::validityCheck(cNode, validAttr);

    std::string id_str = xmlUtil::getAttrValue_string(cNode, "id");
    std::string type_str = xmlUtil::getAttrValue_string(cNode, "type");
    std::string routeID_str = xmlUtil::getAttrValue_string(cNode, "route", false, "");
    std::string from_str = xmlUtil::getAttrValue_string(cNode, "from", false, "");
    std::string to_str = xmlUtil::getAttrValue_string(cNode, "to", false, "");
    std::vector<std::string> via_str_tokenize = xmlUtil::getAttrValue_stringVector(cNode, "via", false, std::vector<std::string>());
    std::string color_str = xmlUtil::getAttrValue_string(cNode, "color", false, "yellow");
    double depart = xmlUtil::getAttrValue_double(cNode, "depart", false, 0);
    int departLane = xmlUtil::getAttrValue_int(cNode, "departLane", false, -5 /*DEPART_LANE_BEST_FREE*/);
    double departPos = xmlUtil::getAttrValue_double(cNode, "departPos", false, 0);
    double departSpeed = xmlUtil::getAttrValue_double(cNode, "departSpeed", false, 0);
    int laneChangeMode = xmlUtil::getAttrValue_int(cNode, "laneChangeMode", false, LANECHANGEMODE_DEFAULT);
    std::string status_str = xmlUtil::getAttrValue_string(cNode, "status", false, "");
    double duration = xmlUtil::getAttrValue_double(cNode, "duration", false, -1);
    double DSRCprob = xmlUtil::getAttrValue_double(cNode, "DSRCprob", false, -1);

    if( !cNode->first_attribute("route") && !cNode->first_attribute("from") && !cNode->first_attribute("to") )
        throw omnetpp::cRuntimeError("either 'route' or 'from/to' attributes should be defined in element '%s'", vehicle_tag.c_str());

    if( cNode->first_attribute("route") && (cNode->first_attribute("from") || cNode->first_attribute("to")) )
        throw omnetpp::cRuntimeError("attribute 'from/to' is redundant when 'route' is present in element '%s'", vehicle_tag.c_str());

    if( !cNode->first_attribute("route") && (!cNode->first_attribute("from") || !cNode->first_attribute("to")) )
        throw omnetpp::cRuntimeError("attribute 'from/to' should be both present in element '%s'", vehicle_tag.c_str());

    if(depart < 0)
        throw omnetpp::cRuntimeError("attribute 'depart' is negative in element '%s': %f", vehicle_tag.c_str(), depart);

    if(laneChangeMode < 0)
        throw omnetpp::cRuntimeError("attribute 'laneChangeMode' is not valid in element '%s': %d", vehicle_tag.c_str(), laneChangeMode);

    if(status_str != "" && status_str != "stopped" && status_str != "parked")
        throw omnetpp::cRuntimeError("attribute 'status' is unknown in element '%s': %s", vehicle_tag.c_str(), status_str.c_str());

    if(cNode->first_attribute("duration") && !cNode->first_attribute("status"))
        throw omnetpp::cRuntimeError("attribute 'status' is required when 'duration' is present in element '%s'", vehicle_tag.c_str());

    if(DSRCprob != -1 && (DSRCprob < 0 || DSRCprob > 1))
        throw omnetpp::cRuntimeError("attribute 'DSRCprob' is not valid in element '%s'", vehicle_tag.c_str());

    auto it = allVehicle.find(id_str);
    if(it == allVehicle.end())
    {
        vehicleEntry_t entry = {};

        entry.id_str = id_str;
        entry.type_str = type_str;
        entry.routeID_str = routeID_str;
        entry.from_str = from_str;
        entry.to_str = to_str;
        entry.via_str_tokenize = via_str_tokenize;
        entry.color_str = color_str;
        entry.depart = depart;
        entry.departLane = departLane;
        entry.departPos = departPos;
        entry.departSpeed = departSpeed;
        entry.laneChangeMode = laneChangeMode;
        entry.status_str = status_str;
        entry.duration = duration;
        entry.DSRCprob = DSRCprob;

        allVehicle.insert(std::make_pair(id_str, entry));
    }
    else
        throw omnetpp::cRuntimeError("Multiple '%s' with the same 'id' %s is not allowed!", vehicle_tag.c_str(), id_str.c_str());
}


void AddNode::addVehicle()
{
    if(allVehicle.empty())
        return;

    unsigned int num = allVehicle.size();

    LOG_DEBUG << boost::format("\n>>> AddNode is adding %1% vehicle modules ... \n") % num << std::flush;

    const char* seed_s = omnetpp::getEnvir()->getConfigEx()->getVariable(CFGVAR_RUNNUMBER);
    int seed = atoi(seed_s);

    // mersenne twister engine -- choose a fix seed to make tests reproducible
    std::mt19937 generator(seed);

    // generating a random floating point number uniformly in [1,0)
    std::uniform_real_distribution<> DSRC_Dist(0,1);

    for(auto &entry : allVehicle)
    {
        std::string vehID = entry.second.id_str;
        std::string route_str = entry.second.routeID_str;
        std::string from_str = entry.second.from_str;
        std::string to_str = entry.second.to_str;

        std::string vehRouteID = "";
        if(route_str != "") vehRouteID = route_str;
        else if(from_str != "" && to_str != "")
        {
            // append 'from_str', 'via_str' and 'to_str'
            std::vector<std::string> allEdges = {from_str};
            allEdges.insert(allEdges.end(), entry.second.via_str_tokenize.begin(), entry.second.via_str_tokenize.end());
            allEdges.push_back(to_str);

            vehRouteID = getShortestRoute(allEdges);
        }
        else
            throw omnetpp::cRuntimeError("'route' or 'from/to' attributes should be defined in element '%s'", vehicle_tag.c_str());

        // now we add a vehicle
        TraCI->vehicleAdd(vehID,
                entry.second.type_str,
                vehRouteID,
                (int32_t)(entry.second.depart*1000),
                entry.second.departPos,
                entry.second.departSpeed,
                entry.second.departLane);

        if(entry.second.status_str == "stopped")
        {
            TraCI->vehicleSetSpeed(vehID, 0);

            // no lane change during stopped
            TraCI->vehicleSetLaneChangeMode(vehID, LANECHANGEMODE_STOPPED);

            if(entry.second.duration != -1)
            {
                omnetpp::cMessage* evt = new omnetpp::cMessage(vehID.c_str(), TYPE_TIMER_STOPPED_VEHICLE);
                scheduleAt(omnetpp::simTime() + 2*SUMO_timeStep + entry.second.depart + entry.second.duration, evt);
            }
        }
        else if(entry.second.status_str == "parked")
        {
            // get all edges for this route
            std::vector<std::string> edges = TraCI->routeGetEdges(vehRouteID);

            double duration = (entry.second.duration != -1) ? 1000*entry.second.duration : std::numeric_limits<int32_t>::max();

            TraCI->vehicleSetStop(vehID, edges[0], entry.second.departPos, entry.second.departLane, (int32_t)duration, 1 /*parking*/);
        }
        else
        {
            // change lane change mode
            if(entry.second.laneChangeMode != LANECHANGEMODE_DEFAULT)
                TraCI->vehicleSetLaneChangeMode(vehID, entry.second.laneChangeMode);
        }

        // and change its color
        RGB newColor = Color::colorNameToRGB(entry.second.color_str);
        TraCI->vehicleSetColor(vehID, newColor);

        int DSRC_status = -1;
        if(entry.second.DSRCprob != -1)
        {
            double rnd_type = DSRC_Dist(generator);
            if(rnd_type >= 0 && rnd_type < entry.second.DSRCprob)
                DSRC_status = 1;
            else
                DSRC_status = 0;

            veh_deferred_attributes_t deferred_entry;
            deferred_entry.DSRC_status = DSRC_status;
            addDeferredAttribute(vehID, deferred_entry);
        }
    }
}


// calculates the shortest route that passes through all edges
std::string AddNode::getShortestRoute(std::vector<std::string> edges)
{
    static std::map<std::string /*routeID*/, std::vector<std::string> /*route edges*/> subRoutes;
    static std::vector<std::string> defined_routes;

    std::ostringstream routeName;
    for(auto &edge : edges)
        routeName << edge << "_";
    std::string routeName_str = routeName.str();

    // is this route defined in SUMO before?
    auto it = std::find(defined_routes.begin(), defined_routes.end(), routeName_str);
    if(it != defined_routes.end())
        return *it;

    std::vector<std::string> finalRoute;
    for(unsigned int i = 0; i < edges.size()-1; i++)
    {
        // construct the route ID
        std::ostringstream routeName;
        routeName << boost::format("%s_%s") % edges[i] % edges[i+1];
        std::string routeName_str = routeName.str();

        // did we calculate the route before?
        auto it = subRoutes.find(routeName_str);
        if(it != subRoutes.end())
        {
            if(!finalRoute.empty())
                finalRoute.pop_back();

            finalRoute.insert(finalRoute.end(), it->second.begin(), it->second.end());
        }
        else
        {
            // get the shortest route between these two edges
            auto shortest_route = TraCI->routeShortest(edges[i], edges[i+1]);
            routeCalculation++;

            if(!finalRoute.empty())
                finalRoute.pop_back();

            finalRoute.insert(finalRoute.end(), shortest_route.begin(), shortest_route.end());

            // push it into allRouteIDs
            subRoutes[routeName_str] = shortest_route;
        }
    }

    // define it in SUMO
    TraCI->routeAdd(routeName_str, finalRoute);

    defined_routes.push_back(routeName_str);

    return routeName_str;
}


void AddNode::parseVehicleFlow(rapidxml::xml_node<> *cNode)
{
    ASSERT(std::string(cNode->name()) == vehicle_flow_tag);

    std::vector<std::string> validAttr = {"id", "type", "typeDist", "color", "route", "from", "to", "via", "departLane",
            "departPos", "departSpeed", "laneChangeMode", "number", "begin", "end", "distribution",
            "period", "lambda", "seed", "probability", "DSRCprob"};
    xmlUtil::validityCheck(cNode, validAttr);

    std::string id_str = xmlUtil::getAttrValue_string(cNode, "id");
    std::vector<std::string> type_str_tokenize = xmlUtil::getAttrValue_stringVector(cNode, "type");
    std::vector<double> typeDist_tokenize = xmlUtil::getAttrValue_doubleVector(cNode, "typeDist", false, std::vector<double>());
    std::string routeID_str = xmlUtil::getAttrValue_string(cNode, "route", false, "");
    std::string from_str = xmlUtil::getAttrValue_string(cNode, "from", false, "");
    std::string to_str = xmlUtil::getAttrValue_string(cNode, "to", false, "");
    std::vector<std::string> via_str_tokenize = xmlUtil::getAttrValue_stringVector(cNode, "via", false, std::vector<std::string>());
    std::string color_str = xmlUtil::getAttrValue_string(cNode, "color", false, "yellow");
    int departLane = xmlUtil::getAttrValue_int(cNode, "departLane", false, -5 /*DEPART_LANE_BEST_FREE*/);
    double departPos = xmlUtil::getAttrValue_double(cNode, "departPos", false, 0);
    double departSpeed = xmlUtil::getAttrValue_double(cNode, "departSpeed", false, 0);
    int laneChangeMode = xmlUtil::getAttrValue_int(cNode, "laneChangeMode", false, LANECHANGEMODE_DEFAULT);
    double begin = xmlUtil::getAttrValue_double(cNode, "begin", false, 0);
    int number = xmlUtil::getAttrValue_int(cNode, "number", false, -1);
    double end = xmlUtil::getAttrValue_double(cNode, "end", false, -1);
    int seed = xmlUtil::getAttrValue_int(cNode, "seed", false, 0);
    std::string distribution_str = xmlUtil::getAttrValue_string(cNode, "distribution");
    double period = xmlUtil::getAttrValue_double(cNode, "period", false, -1);
    double lambda = xmlUtil::getAttrValue_double(cNode, "lambda", false, -1);
    double probability = xmlUtil::getAttrValue_double(cNode, "probability", false, -1);
    double DSRCprob = xmlUtil::getAttrValue_double(cNode, "DSRCprob", false, -1);

    // we have multiple types
    if(type_str_tokenize.size() > 1)
    {
        if(type_str_tokenize.size() != typeDist_tokenize.size())
            throw omnetpp::cRuntimeError("'type' and 'typeDist' attributes do not match in element '%s'", vehicle_flow_tag.c_str());

        double sum = 0;
        for(auto &entry : typeDist_tokenize)
            sum += entry;

        if(sum != 100)
            throw omnetpp::cRuntimeError("'typeDist' values do not add up to 100 percent in element '%s'", vehicle_flow_tag.c_str());
    }

    if( !cNode->first_attribute("route") && !cNode->first_attribute("from") && !cNode->first_attribute("to") )
        throw omnetpp::cRuntimeError("either 'route' or 'from/to' attributes should be defined in element '%s'", vehicle_flow_tag.c_str());

    if( cNode->first_attribute("route") && (cNode->first_attribute("from") || cNode->first_attribute("to")) )
        throw omnetpp::cRuntimeError("attribute 'from/to' is redundant when 'route' is present in element '%s'", vehicle_flow_tag.c_str());

    if( !cNode->first_attribute("route") && (!cNode->first_attribute("from") || !cNode->first_attribute("to")) )
        throw omnetpp::cRuntimeError("attribute 'from/to' should be both present in element '%s'", vehicle_flow_tag.c_str());

    if(laneChangeMode < 0)
        throw omnetpp::cRuntimeError("attribute 'laneChangeMode' is not valid in element '%s': %d", vehicle_flow_tag.c_str(), laneChangeMode);

    if(begin < 0)
        throw omnetpp::cRuntimeError("attribute 'begin' is negative in element '%s': %d", vehicle_flow_tag.c_str(), begin);

    if(number != -1 && number < 0)
        throw omnetpp::cRuntimeError("'number' value is negative in element '%s': %d", vehicle_flow_tag.c_str(), number);

    if(cNode->first_attribute("number") && cNode->first_attribute("end"))
        throw omnetpp::cRuntimeError("either 'end' or 'number' should be present in element '%s'", vehicle_flow_tag.c_str());

    if(end != -1 && end <= begin)
        throw omnetpp::cRuntimeError("'end' value is smaller than 'begin' value in element '%s'", vehicle_flow_tag.c_str());

    if(seed < 0)
        throw omnetpp::cRuntimeError("'seed' value should be positive in element '%s': %d", vehicle_flow_tag.c_str(), seed);

    if(distribution_str != "deterministic" && distribution_str != "poisson" && distribution_str != "uniform")
        throw omnetpp::cRuntimeError("'distribution' value is invalid in element '%s': %s", vehicle_flow_tag.c_str(), distribution_str.c_str());

    // period can be zero too!
    if(period != -1 && period < 0)
        throw omnetpp::cRuntimeError("'period' value is negative in element '%s': %f", vehicle_flow_tag.c_str(), period);

    if(lambda != -1 && lambda <= 0)
        throw omnetpp::cRuntimeError("'lambda' value should be positive in element '%s': %s", vehicle_flow_tag.c_str(), lambda);

    if(probability != -1 && (probability < 0 || probability > 1))
        throw omnetpp::cRuntimeError("'probability' should be in range [0,1] in element '%s': %f", vehicle_flow_tag.c_str(), probability);

    if(distribution_str == "deterministic" && !cNode->first_attribute("period"))
        throw omnetpp::cRuntimeError("attribute 'period' is not found in element '%s'", vehicle_flow_tag.c_str());

    if(distribution_str == "deterministic" && (cNode->first_attribute("lambda") || cNode->first_attribute("probability")))
        throw omnetpp::cRuntimeError("attribute 'lambda/probability' is redundant in deterministic distribution in element '%s'", vehicle_flow_tag.c_str());

    if(distribution_str == "poisson" && !cNode->first_attribute("lambda"))
        throw omnetpp::cRuntimeError("attribute 'lambda' is not found in element '%s'", vehicle_flow_tag.c_str());

    if(distribution_str == "poisson" && (cNode->first_attribute("period") || cNode->first_attribute("probability")))
        throw omnetpp::cRuntimeError("attribute 'period/probability' is redundant in poisson distribution in element '%s'", vehicle_flow_tag.c_str());

    if(distribution_str == "uniform" && (cNode->first_attribute("period") || cNode->first_attribute("lambda")))
        throw omnetpp::cRuntimeError("attribute 'period/lambda' is redundant in uniform distribution in element '%s'", vehicle_flow_tag.c_str());

    if(distribution_str == "uniform" && !cNode->first_attribute("probability"))
        throw omnetpp::cRuntimeError("attribute 'probability' is not found in element '%s'", vehicle_flow_tag.c_str());

    if(DSRCprob != -1 && (DSRCprob < 0 || DSRCprob > 1))
        throw omnetpp::cRuntimeError("attribute 'DSRCprob' is not valid in element '%s'", vehicle_flow_tag.c_str());

    if(end == -1)
    {
        if(terminateTime != -1)
            end = terminateTime;
        else
            end = std::numeric_limits<int32_t>::max();
    }
    else
    {
        if(terminateTime != -1)
            end = std::min(end, terminateTime);
    }

    auto it = allVehicleFlow.find(id_str);
    if(it == allVehicleFlow.end())
    {
        vehicleFlowEntry_t entry = {};

        entry.id_str = id_str;
        entry.type_str_tokenize = type_str_tokenize;
        entry.typeDist_tokenize = typeDist_tokenize;
        entry.routeID_str = routeID_str;
        entry.from_str = from_str;
        entry.to_str = to_str;
        entry.via_str_tokenize = via_str_tokenize;
        entry.color_str = color_str;
        entry.departLane = departLane;
        entry.departPos = departPos;
        entry.departSpeed = departSpeed;
        entry.laneChangeMode = laneChangeMode;
        entry.number = number;
        entry.begin = begin;
        entry.end = end;
        entry.seed = seed;
        entry.distribution_str = distribution_str;
        entry.period = period;
        entry.lambda = lambda;
        entry.probability = probability;
        entry.DSRCprob = DSRCprob;

        allVehicleFlow.insert(std::make_pair(id_str, entry));
    }
    else
        throw omnetpp::cRuntimeError("Multiple '%s' with the same 'id' %s is not allowed!", vehicle_flow_tag.c_str(), id_str.c_str());
}


void AddNode::addVehicleFlow()
{
    if(allVehicleFlow.empty())
        return;

    unsigned int num = allVehicleFlow.size();

    LOG_DEBUG << boost::format("\n>>> AddNode is adding %1% vehicle flows ... \n") % num << std::flush;

    // iterate over each flow
    for(auto &entry : allVehicleFlow)
    {
        if(entry.second.end <= entry.second.begin)
            continue;

        // each flow has its own seed/generator
        // mersenne twister engine -- choose a fix seed to make tests reproducible
        std::mt19937 generator(entry.second.seed);

        // generating a random floating point number uniformly in [1,0)
        std::uniform_real_distribution<> vehTypeDist(0,1);

        // generating a random floating point number uniformly in [1,0)
        std::uniform_real_distribution<> DSRC_Dist(0,1);

        int maxVehNum = (entry.second.number != -1) ? entry.second.number : std::numeric_limits<int>::max();

        std::string route_str = entry.second.routeID_str;
        std::string from_str = entry.second.from_str;
        std::string to_str = entry.second.to_str;

        std::string vehRouteID = "";
        if(route_str != "") vehRouteID = route_str;
        else if(from_str != "" && to_str != "")
        {
            // append 'from_str', 'via_str' and 'to_str'
            std::vector<std::string> allEdges = {from_str};
            allEdges.insert(allEdges.end(), entry.second.via_str_tokenize.begin(), entry.second.via_str_tokenize.end());
            allEdges.push_back(to_str);

            vehRouteID = getShortestRoute(allEdges);
        }
        else
            throw omnetpp::cRuntimeError("'route' or 'from/to' attributes should be defined in node '%s'", vehicle_flow_tag.c_str());

        if(entry.second.distribution_str == "deterministic")
        {
            double depart = entry.second.begin;

            // for each vehicle
            for(int veh = 0; veh < maxVehNum; veh++)
            {
                if(depart >= entry.second.end)
                    break;

                std::string vehID = entry.second.id_str + "." + std::to_string(veh);

                std::string vehType = entry.second.type_str_tokenize[0];
                if(entry.second.type_str_tokenize.size() > 1)
                {
                    double rnd_type = vehTypeDist(generator);
                    vehType = getVehType(entry.second, rnd_type);
                }

                // now we add a vehicle as obstacle
                TraCI->vehicleAdd(vehID, vehType, vehRouteID, (int32_t)(depart*1000), entry.second.departPos, entry.second.departSpeed, entry.second.departLane);

                // change its color
                RGB newColor = Color::colorNameToRGB(entry.second.color_str);
                TraCI->vehicleSetColor(vehID, newColor);

                // change lane change mode
                if(entry.second.laneChangeMode != LANECHANGEMODE_DEFAULT)
                    TraCI->vehicleSetLaneChangeMode(vehID, entry.second.laneChangeMode);

                depart += entry.second.period;

                int DSRC_status = -1;
                if(entry.second.DSRCprob != -1)
                {
                    double rnd_type = DSRC_Dist(generator);
                    if(rnd_type >= 0 && rnd_type < entry.second.DSRCprob)
                        DSRC_status = 1;
                    else
                        DSRC_status = 0;

                    veh_deferred_attributes_t deferred_entry;
                    deferred_entry.DSRC_status = DSRC_status;
                    addDeferredAttribute(vehID, deferred_entry);
                }
            }
        }
        else if(entry.second.distribution_str == "poisson")
        {
            // change unit from veh/h to veh/TS (TS is the SUMO time step)
            double lambda = (entry.second.lambda * SUMO_timeStep) / 3600.;

            // poisson distribution with rate lambda
            std::poisson_distribution<long> arrivalDist(lambda);

            // how many vehicles are inserted until now
            int vehCount = 0;

            // on each SUMO time step
            for(double depart = entry.second.begin ; depart < entry.second.end; depart += SUMO_timeStep)
            {
                // # vehicles inserted in each second
                int vehInsert = arrivalDist(generator);

                for(int veh = 1; veh <= vehInsert; ++veh)
                {
                    std::string vehID = entry.second.id_str + "." + std::to_string(vehCount);

                    std::string vehType = entry.second.type_str_tokenize[0];
                    if(entry.second.type_str_tokenize.size() > 1)
                    {
                        double rnd_type = vehTypeDist(generator);
                        vehType = getVehType(entry.second, rnd_type);
                    }

                    // now we add a vehicle
                    TraCI->vehicleAdd(vehID, vehType, vehRouteID, (int32_t)(depart*1000), entry.second.departPos, entry.second.departSpeed, entry.second.departLane);

                    // change its color
                    RGB newColor = Color::colorNameToRGB(entry.second.color_str);
                    TraCI->vehicleSetColor(vehID, newColor);

                    // change lane change mode
                    if(entry.second.laneChangeMode != LANECHANGEMODE_DEFAULT)
                        TraCI->vehicleSetLaneChangeMode(vehID, entry.second.laneChangeMode);

                    int DSRC_status = -1;
                    if(entry.second.DSRCprob != -1)
                    {
                        double rnd_type = DSRC_Dist(generator);
                        if(rnd_type >= 0 && rnd_type < entry.second.DSRCprob)
                            DSRC_status = 1;
                        else
                            DSRC_status = 0;

                        veh_deferred_attributes_t deferred_entry;
                        deferred_entry.DSRC_status = DSRC_status;
                        addDeferredAttribute(vehID, deferred_entry);
                    }

                    vehCount++;

                    if(vehCount >= maxVehNum)
                        break;
                }

                if(vehCount >= maxVehNum)
                    break;
            }
        }
        else if(entry.second.distribution_str == "uniform")
        {
            // generating a random floating point number uniformly in [1,0)
            std::uniform_real_distribution<> vehDeparture(0,1);

            // how many vehicles are inserted until now
            int vehCount = 0;

            // on each second (not each time step)
            for(double depart = entry.second.begin; depart < entry.second.end; depart ++)
            {
                // should we depart this vehicle?
                double rnd_type = vehDeparture(generator);

                if(rnd_type >= 0 && rnd_type < entry.second.probability)
                {
                    std::string vehID = entry.second.id_str + "." + std::to_string(vehCount);

                    std::string vehType = entry.second.type_str_tokenize[0];
                    if(entry.second.type_str_tokenize.size() > 1)
                    {
                        double rnd_type = vehTypeDist(generator);
                        vehType = getVehType(entry.second, rnd_type);
                    }

                    // now we add a vehicle
                    TraCI->vehicleAdd(vehID, vehType, vehRouteID, (int32_t)(depart*1000), entry.second.departPos, entry.second.departSpeed, entry.second.departLane);

                    // change its color
                    RGB newColor = Color::colorNameToRGB(entry.second.color_str);
                    TraCI->vehicleSetColor(vehID, newColor);

                    // change lane change mode
                    if(entry.second.laneChangeMode != LANECHANGEMODE_DEFAULT)
                        TraCI->vehicleSetLaneChangeMode(vehID, entry.second.laneChangeMode);

                    int DSRC_status = -1;
                    if(entry.second.DSRCprob != -1)
                    {
                        double rnd_type = DSRC_Dist(generator);
                        if(rnd_type >= 0 && rnd_type < entry.second.DSRCprob)
                            DSRC_status = 1;
                        else
                            DSRC_status = 0;

                        veh_deferred_attributes_t deferred_entry;
                        deferred_entry.DSRC_status = DSRC_status;
                        addDeferredAttribute(vehID, deferred_entry);
                    }

                    vehCount++;

                    if(vehCount >= maxVehNum)
                        break;
                }

                if(vehCount >= maxVehNum)
                    break;
            }
        }
    }
}


std::string AddNode::getVehType(vehicleFlowEntry_t entry, double rnd)
{
    std::string vehType = "";
    double lowerBound = 0;
    double upperBound = entry.typeDist_tokenize[0]/100.;

    for(unsigned int i = 0; i < entry.typeDist_tokenize.size(); i++)
    {
        if(rnd >= lowerBound && rnd < upperBound)
        {
            vehType = entry.type_str_tokenize[i];
            break;
        }

        lowerBound += entry.typeDist_tokenize[i]/100.;
        upperBound += entry.typeDist_tokenize[i+1]/100.;
    }

    if(vehType == "")
        throw omnetpp::cRuntimeError("vehType cannot be empty");

    return vehType;
}


void AddNode::parseVehicleMultiFlow(rapidxml::xml_node<> *cNode)
{
    ASSERT(std::string(cNode->name()) == vehicle_multiFlow_tag);

    std::vector<std::string> validAttr = {"id", "type", "typeDist", "route", "routeDist", "color", "departLane",
            "departPos", "departSpeed", "laneChangeMode", "begin", "number", "end", "distribution",
            "period", "lambda", "seed", "probability", "DSRCprob"};
    xmlUtil::validityCheck(cNode, validAttr);

    std::string id_str = xmlUtil::getAttrValue_string(cNode, "id");
    std::vector<std::string> type_str_tokenize = xmlUtil::getAttrValue_stringVector(cNode, "type");
    std::vector<double> typeDist_tokenize = xmlUtil::getAttrValue_doubleVector(cNode, "typeDist", false, std::vector<double>());
    // route is mandatory here
    std::vector<std::string> routeID_str_tokenize = xmlUtil::getAttrValue_stringVector(cNode, "route");
    // we now have route distribution
    std::vector<double> routeDist_tokenize = xmlUtil::getAttrValue_doubleVector(cNode, "routeDist", false, std::vector<double>());
    std::string color_str = xmlUtil::getAttrValue_string(cNode, "color", false, "yellow");
    int departLane = xmlUtil::getAttrValue_int(cNode, "departLane", false, -5 /*DEPART_LANE_BEST_FREE*/);
    double departPos = xmlUtil::getAttrValue_double(cNode, "departPos", false, 0);
    double departSpeed = xmlUtil::getAttrValue_double(cNode, "departSpeed", false, 0);
    int laneChangeMode = xmlUtil::getAttrValue_int(cNode, "laneChangeMode", false, LANECHANGEMODE_DEFAULT);
    double begin = xmlUtil::getAttrValue_double(cNode, "begin", false, 0);
    int number = xmlUtil::getAttrValue_int(cNode, "number", false, -1);
    double end = xmlUtil::getAttrValue_double(cNode, "end", false, -1);
    int seed = xmlUtil::getAttrValue_int(cNode, "seed", false, 0);
    std::string distribution_str = xmlUtil::getAttrValue_string(cNode, "distribution");
    double period = xmlUtil::getAttrValue_double(cNode, "period", false, -1);
    double lambda = xmlUtil::getAttrValue_double(cNode, "lambda", false, -1);
    double probability = xmlUtil::getAttrValue_double(cNode, "probability", false, -1);
    double DSRCprob = xmlUtil::getAttrValue_double(cNode, "DSRCprob", false, -1);

    // we have multiple types
    if(type_str_tokenize.size() > 1)
    {
        if(type_str_tokenize.size() != typeDist_tokenize.size())
            throw omnetpp::cRuntimeError("'type' and 'typeDist' attributes do not match in element '%s'", vehicle_multiFlow_tag.c_str());

        double sum = 0;
        for(auto &entry : typeDist_tokenize)
            sum += entry;

        if(sum != 100)
            throw omnetpp::cRuntimeError("'typeDist' values do not add up to 100 percent in element '%s'", vehicle_multiFlow_tag.c_str());
    }

    // we have multiple routes
    if(routeID_str_tokenize.size() > 1)
    {
        if(routeID_str_tokenize.size() != routeDist_tokenize.size())
            throw omnetpp::cRuntimeError("'route' and 'routeDist' attributes do not match in element '%s'", vehicle_multiFlow_tag.c_str());

        double sum = 0;
        for(auto &entry : routeDist_tokenize)
            sum += entry;

        if(sum != 100)
            throw omnetpp::cRuntimeError("'routeDist' values do not add up to 100 percent in element '%s'", vehicle_multiFlow_tag.c_str());

        std::string lastFirstEdge = (TraCI->routeGetEdges(routeID_str_tokenize[0]))[0];
        for(unsigned int i = 1; i < routeID_str_tokenize.size(); i++)
        {
            auto firstEdge = (TraCI->routeGetEdges(routeID_str_tokenize[i]))[0];
            if(firstEdge != lastFirstEdge)
                throw omnetpp::cRuntimeError("all routeIDs should have the same starting edge in element '%s'", vehicle_multiFlow_tag.c_str());
        }
    }

    if(laneChangeMode < 0)
        throw omnetpp::cRuntimeError("attribute 'laneChangeMode' is not valid in element '%s': %d", vehicle_multiFlow_tag.c_str(), laneChangeMode);

    if(begin < 0)
        throw omnetpp::cRuntimeError("attribute 'begin' is negative in element '%s': %d", vehicle_multiFlow_tag.c_str(), begin);

    if(number != -1 && number < 0)
        throw omnetpp::cRuntimeError("'number' value is negative in element '%s': %d", vehicle_multiFlow_tag.c_str(), number);

    if(cNode->first_attribute("number") && cNode->first_attribute("end"))
        throw omnetpp::cRuntimeError("either 'end' or 'number' should be present in element '%s'", vehicle_multiFlow_tag.c_str());

    if(end != -1 && end <= begin)
        throw omnetpp::cRuntimeError("'end' value is smaller than 'begin' value in element '%s'", vehicle_multiFlow_tag.c_str());

    if(seed < 0)
        throw omnetpp::cRuntimeError("'seed' value should be positive in element '%s': %d", vehicle_multiFlow_tag.c_str(), seed);

    if(distribution_str != "deterministic" && distribution_str != "poisson" && distribution_str != "uniform")
        throw omnetpp::cRuntimeError("'distribution' value is invalid in element '%s': %s", vehicle_multiFlow_tag.c_str(), distribution_str.c_str());

    // period can be zero too!
    if(period != -1 && period < 0)
        throw omnetpp::cRuntimeError("'period' value is negative in element '%s': %f", vehicle_multiFlow_tag.c_str(), period);

    if(lambda != -1 && lambda <= 0)
        throw omnetpp::cRuntimeError("'lambda' value should be positive in element '%s': %s", vehicle_multiFlow_tag.c_str(), lambda);

    if(probability != -1 && (probability < 0 || probability > 1))
        throw omnetpp::cRuntimeError("'probability' should be in range [0,1] in element '%s': %f", vehicle_multiFlow_tag.c_str(), probability);

    if(distribution_str == "deterministic" && !cNode->first_attribute("period"))
        throw omnetpp::cRuntimeError("attribute 'period' is not found in element '%s'", vehicle_multiFlow_tag.c_str());

    if(distribution_str == "deterministic" && (cNode->first_attribute("lambda") || cNode->first_attribute("probability")))
        throw omnetpp::cRuntimeError("attribute 'lambda/probability' is redundant in deterministic distribution in element '%s'", vehicle_multiFlow_tag.c_str());

    if(distribution_str == "poisson" && !cNode->first_attribute("lambda"))
        throw omnetpp::cRuntimeError("attribute 'lambda' is not found in element '%s'", vehicle_multiFlow_tag.c_str());

    if(distribution_str == "poisson" && (cNode->first_attribute("period") || cNode->first_attribute("probability")))
        throw omnetpp::cRuntimeError("attribute 'period/probability' is redundant in poisson distribution in element '%s'", vehicle_multiFlow_tag.c_str());

    if(distribution_str == "uniform" && (cNode->first_attribute("period") || cNode->first_attribute("lambda")))
        throw omnetpp::cRuntimeError("attribute 'period/lambda' is redundant in uniform distribution in element '%s'", vehicle_multiFlow_tag.c_str());

    if(distribution_str == "uniform" && !cNode->first_attribute("probability"))
        throw omnetpp::cRuntimeError("attribute 'probability' is not found in element '%s'", vehicle_multiFlow_tag.c_str());

    if(DSRCprob != -1 && (DSRCprob < 0 || DSRCprob > 1))
        throw omnetpp::cRuntimeError("attribute 'DSRCprob' is not valid in element '%s'", vehicle_multiFlow_tag.c_str());

    if(end == -1)
    {
        if(terminateTime != -1)
            end = terminateTime;
        else
            end = std::numeric_limits<int32_t>::max();
    }
    else
    {
        if(terminateTime != -1)
            end = std::min(end, terminateTime);
    }

    auto it = allVehicleMultiFlow.find(id_str);
    if(it == allVehicleMultiFlow.end())
    {
        vehicleMultiFlowEntry_t entry = {};

        entry.id_str = id_str;
        entry.type_str_tokenize = type_str_tokenize;
        entry.typeDist_tokenize = typeDist_tokenize;
        entry.color_str = color_str;
        entry.routeID_str_tokenize = routeID_str_tokenize;
        entry.routeDist_tokenize = routeDist_tokenize;
        entry.departLane = departLane;
        entry.departPos = departPos;
        entry.departSpeed = departSpeed;
        entry.laneChangeMode = laneChangeMode;
        entry.number = number;
        entry.begin = begin;
        entry.end = end;
        entry.seed = seed;
        entry.distribution_str = distribution_str;
        entry.period = period;
        entry.lambda = lambda;
        entry.probability = probability;
        entry.DSRCprob = DSRCprob;

        allVehicleMultiFlow.insert(std::make_pair(id_str, entry));
    }
    else
        throw omnetpp::cRuntimeError("Multiple '%s' with the same 'id' %s is not allowed!", vehicle_multiFlow_tag.c_str(), id_str.c_str());
}


void AddNode::addVehicleMultiFlow()
{
    if(allVehicleMultiFlow.empty())
        return;

    unsigned int num = allVehicleMultiFlow.size();

    LOG_DEBUG << boost::format("\n>>> AddNode is adding %1% vehicle multi-flows ... \n") % num << std::flush;

    // iterate over each flow
    for(auto &entry : allVehicleMultiFlow)
    {
        if(entry.second.end <= entry.second.begin)
            continue;

        // each flow has its own seed/generator
        // mersenne twister engine -- choose a fix seed to make tests reproducible
        std::mt19937 generator(entry.second.seed);

        // generating a random floating point number uniformly in [1,0)
        std::uniform_real_distribution<> vehTypeDist(0,1);

        // generating a random floating point number uniformly in [1,0)
        std::uniform_real_distribution<> vehRouteDist(0,1);

        // generating a random floating point number uniformly in [1,0)
        std::uniform_real_distribution<> DSRC_Dist(0,1);

        int maxVehNum = (entry.second.number != -1) ? entry.second.number : std::numeric_limits<int>::max();

        if(entry.second.distribution_str == "deterministic")
        {
            double depart = entry.second.begin;

            // for each vehicle
            for(int veh = 0; veh < maxVehNum; veh++)
            {
                if(depart >= entry.second.end)
                    break;

                std::string vehID = entry.second.id_str + "." + std::to_string(veh);

                std::string vehType = entry.second.type_str_tokenize[0];
                if(entry.second.type_str_tokenize.size() > 1)
                {
                    double rnd_type = vehTypeDist(generator);
                    vehType = getVehType(entry.second, rnd_type);
                }

                std::string vehRoute = entry.second.routeID_str_tokenize[0];
                if(entry.second.routeID_str_tokenize.size() > 1)
                {
                    double rnd_route = vehRouteDist(generator);
                    vehRoute = getVehRoute(entry.second, rnd_route);
                }

                // now we add a vehicle as obstacle
                TraCI->vehicleAdd(vehID, vehType, vehRoute, (int32_t)(depart*1000), entry.second.departPos, entry.second.departSpeed, entry.second.departLane);

                // change its color
                RGB newColor = Color::colorNameToRGB(entry.second.color_str);
                TraCI->vehicleSetColor(vehID, newColor);

                // change lane change mode
                if(entry.second.laneChangeMode != LANECHANGEMODE_DEFAULT)
                    TraCI->vehicleSetLaneChangeMode(vehID, entry.second.laneChangeMode);

                depart += entry.second.period;

                int DSRC_status = -1;
                if(entry.second.DSRCprob != -1)
                {
                    double rnd_type = DSRC_Dist(generator);
                    if(rnd_type >= 0 && rnd_type < entry.second.DSRCprob)
                        DSRC_status = 1;
                    else
                        DSRC_status = 0;

                    veh_deferred_attributes_t deferred_entry;
                    deferred_entry.DSRC_status = DSRC_status;
                    addDeferredAttribute(vehID, deferred_entry);
                }
            }
        }
        else if(entry.second.distribution_str == "poisson")
        {
            // change unit from veh/h to veh/TS (TS is the SUMO time step)
            double lambda = (entry.second.lambda * SUMO_timeStep) / 3600.;

            // poisson distribution with rate lambda
            std::poisson_distribution<long> arrivalDist(lambda);

            // how many vehicles are inserted until now
            int vehCount = 0;

            // on each SUMO time step
            for(double depart = entry.second.begin ; depart < entry.second.end; depart += SUMO_timeStep)
            {
                // # vehicles inserted in each second
                int vehInsert = arrivalDist(generator);

                for(int veh = 1; veh <= vehInsert; ++veh)
                {
                    std::string vehID = entry.second.id_str + "." + std::to_string(vehCount);

                    std::string vehType = entry.second.type_str_tokenize[0];
                    if(entry.second.type_str_tokenize.size() > 1)
                    {
                        double rnd_type = vehTypeDist(generator);
                        vehType = getVehType(entry.second, rnd_type);
                    }

                    std::string vehRoute = entry.second.routeID_str_tokenize[0];
                    if(entry.second.routeID_str_tokenize.size() > 1)
                    {
                        double rnd_route = vehRouteDist(generator);
                        vehRoute = getVehRoute(entry.second, rnd_route);
                    }

                    // now we add a vehicle
                    TraCI->vehicleAdd(vehID, vehType, vehRoute, (int32_t)(depart*1000), entry.second.departPos, entry.second.departSpeed, entry.second.departLane);

                    // change its color
                    RGB newColor = Color::colorNameToRGB(entry.second.color_str);
                    TraCI->vehicleSetColor(vehID, newColor);

                    // change lane change mode
                    if(entry.second.laneChangeMode != LANECHANGEMODE_DEFAULT)
                        TraCI->vehicleSetLaneChangeMode(vehID, entry.second.laneChangeMode);

                    int DSRC_status = -1;
                    if(entry.second.DSRCprob != -1)
                    {
                        double rnd_type = DSRC_Dist(generator);
                        if(rnd_type >= 0 && rnd_type < entry.second.DSRCprob)
                            DSRC_status = 1;
                        else
                            DSRC_status = 0;

                        veh_deferred_attributes_t deferred_entry;
                        deferred_entry.DSRC_status = DSRC_status;
                        addDeferredAttribute(vehID, deferred_entry);
                    }

                    vehCount++;

                    if(vehCount >= maxVehNum)
                        break;
                }

                if(vehCount >= maxVehNum)
                    break;
            }
        }
        else if(entry.second.distribution_str == "uniform")
        {
            // generating a random floating point number uniformly in [1,0)
            std::uniform_real_distribution<> vehDeparture(0,1);

            // how many vehicles are inserted until now
            int vehCount = 0;

            // on each second (not each time step)
            for(double depart = entry.second.begin; depart < entry.second.end; depart ++)
            {
                // should we depart this vehicle?
                double rnd_type = vehDeparture(generator);

                if(rnd_type >= 0 && rnd_type < entry.second.probability)
                {
                    std::string vehID = entry.second.id_str + "." + std::to_string(vehCount);

                    std::string vehType = entry.second.type_str_tokenize[0];
                    if(entry.second.type_str_tokenize.size() > 1)
                    {
                        double rnd_type = vehTypeDist(generator);
                        vehType = getVehType(entry.second, rnd_type);
                    }

                    std::string vehRoute = entry.second.routeID_str_tokenize[0];
                    if(entry.second.routeID_str_tokenize.size() > 1)
                    {
                        double rnd_route = vehRouteDist(generator);
                        vehRoute = getVehRoute(entry.second, rnd_route);
                    }

                    // now we add a vehicle
                    TraCI->vehicleAdd(vehID, vehType, vehRoute, (int32_t)(depart*1000), entry.second.departPos, entry.second.departSpeed, entry.second.departLane);

                    // change its color
                    RGB newColor = Color::colorNameToRGB(entry.second.color_str);
                    TraCI->vehicleSetColor(vehID, newColor);

                    // change lane change mode
                    if(entry.second.laneChangeMode != LANECHANGEMODE_DEFAULT)
                        TraCI->vehicleSetLaneChangeMode(vehID, entry.second.laneChangeMode);

                    int DSRC_status = -1;
                    if(entry.second.DSRCprob != -1)
                    {
                        double rnd_type = DSRC_Dist(generator);
                        if(rnd_type >= 0 && rnd_type < entry.second.DSRCprob)
                            DSRC_status = 1;
                        else
                            DSRC_status = 0;

                        veh_deferred_attributes_t deferred_entry;
                        deferred_entry.DSRC_status = DSRC_status;
                        addDeferredAttribute(vehID, deferred_entry);
                    }

                    vehCount++;

                    if(vehCount >= maxVehNum)
                        break;
                }

                if(vehCount >= maxVehNum)
                    break;
            }
        }
    }
}


std::string AddNode::getVehRoute(vehicleMultiFlowEntry_t entry, double rnd)
{
    std::string vehRoute = "";
    double lowerBound = 0;
    double upperBound = entry.routeDist_tokenize[0]/100.;

    for(unsigned int i = 0; i < entry.routeDist_tokenize.size(); i++)
    {
        if(rnd >= lowerBound && rnd < upperBound)
        {
            vehRoute = entry.routeID_str_tokenize[i];
            break;
        }

        lowerBound += entry.routeDist_tokenize[i]/100.;
        upperBound += entry.routeDist_tokenize[i+1]/100.;
    }

    if(vehRoute == "")
        throw omnetpp::cRuntimeError("vehRoute cannot be empty");

    return vehRoute;
}


void AddNode::parseVehiclePlatoon(rapidxml::xml_node<> *cNode)
{
    ASSERT(std::string(cNode->name()) == vehicle_platoon_tag);

    std::vector<std::string> validAttr = {"id", "type", "size", "route", "from", "to", "via", "color",
            "depart", "departLane", "departPos", "platoonMaxSpeed", "fastCatchUp", "interGap", "pltMgmtProt", "maxSize", "optSize"};
    xmlUtil::validityCheck(cNode, validAttr);

    std::string id_str = xmlUtil::getAttrValue_string(cNode, "id");
    std::string type_str = xmlUtil::getAttrValue_string(cNode, "type");
    int size = xmlUtil::getAttrValue_int(cNode, "size");
    std::string routeID_str = xmlUtil::getAttrValue_string(cNode, "route", false, "");
    std::string from_str = xmlUtil::getAttrValue_string(cNode, "from", false, "");
    std::string to_str = xmlUtil::getAttrValue_string(cNode, "to", false, "");
    std::vector<std::string> via_str_tokenize = xmlUtil::getAttrValue_stringVector(cNode, "via", false, std::vector<std::string>());
    std::string color_str = xmlUtil::getAttrValue_string(cNode, "color", false, "yellow");
    double depart = xmlUtil::getAttrValue_double(cNode, "depart", false, 0);
    int departLane = xmlUtil::getAttrValue_int(cNode, "departLane", false, 0); // default cannot be negative!
    double departPos = xmlUtil::getAttrValue_double(cNode, "departPos", false, -1);
    double platoonMaxSpeed = xmlUtil::getAttrValue_double(cNode, "platoonMaxSpeed", false, 10);
    bool fastCatchUp = xmlUtil::getAttrValue_bool(cNode, "fastCatchUp", false, false);
    double interGap = xmlUtil::getAttrValue_double(cNode, "interGap", false, -1);
    bool pltMgmtProt = xmlUtil::getAttrValue_bool(cNode, "pltMgmtProt", false, false);
    int maxSize = xmlUtil::getAttrValue_int(cNode, "maxSize", false, -1);
    int optSize = xmlUtil::getAttrValue_int(cNode, "optSize", false, -1);

    if(size < 1)
        throw omnetpp::cRuntimeError("attribute 'size' is invalid in element '%s': %d", vehicle_platoon_tag.c_str(), size);

    if(maxSize != -1 && maxSize < size)
        throw omnetpp::cRuntimeError("attribute 'maxSize' is smaller than 'size' in element '%s': %d", vehicle_platoon_tag.c_str(), maxSize);

    if( !cNode->first_attribute("route") && !cNode->first_attribute("from") && !cNode->first_attribute("to") )
        throw omnetpp::cRuntimeError("either 'route' or 'from/to' attributes should be defined in element '%s'", vehicle_platoon_tag.c_str());

    if( cNode->first_attribute("route") && (cNode->first_attribute("from") || cNode->first_attribute("to")) )
        throw omnetpp::cRuntimeError("attribute 'from/to' is redundant when 'route' is present in element '%s'", vehicle_platoon_tag.c_str());

    if( !cNode->first_attribute("route") && (!cNode->first_attribute("from") || !cNode->first_attribute("to")) )
        throw omnetpp::cRuntimeError("attribute 'from/to' should be both present in element '%s'", vehicle_platoon_tag.c_str());

    if(depart < 0)
        throw omnetpp::cRuntimeError("attribute 'depart' is negative in element '%s': %f", vehicle_platoon_tag.c_str(), depart);

    if(departPos != -1 && departPos < 0)
        throw omnetpp::cRuntimeError("attribute 'departPos' is negative in element '%s': %f", vehicle_platoon_tag.c_str(), departPos);

    if( (cNode->first_attribute("maxSize") || cNode->first_attribute("optSize")) && !cNode->first_attribute("pltMgmtProt") )
        throw omnetpp::cRuntimeError("attribute 'maxSize/optSize' is only valid when 'pltMgmtProt' is defined in element '%s'", vehicle_platoon_tag.c_str());

    auto it = allVehiclePlatoon.find(id_str);
    if(it == allVehiclePlatoon.end())
    {
        vehiclePlatoonEntry_t entry = {};

        entry.id_str = id_str;
        entry.type_str = type_str;
        entry.size = size;
        entry.routeID_str = routeID_str;
        entry.from_str = from_str;
        entry.to_str = to_str;
        entry.via_str_tokenize = via_str_tokenize;
        entry.color_str = color_str;
        entry.depart = depart;
        entry.departLane = departLane;
        entry.departPos = departPos;
        entry.platoonMaxSpeed = platoonMaxSpeed;
        entry.fastCatchUp = fastCatchUp;
        entry.interGap = interGap;
        entry.pltMgmtProt = pltMgmtProt;
        entry.optSize = optSize;
        entry.maxSize = maxSize;

        allVehiclePlatoon.insert(std::make_pair(id_str, entry));
    }
    else
        throw omnetpp::cRuntimeError("Multiple '%s' with the same 'id' %s is not allowed!", vehicle_platoon_tag.c_str(), id_str.c_str());

    // iterate over children nodes (if exist)
    parseVehiclePlatoonChild(cNode);
}


void AddNode::parseVehiclePlatoonChild(rapidxml::xml_node<> *pNode)
{
    std::string platoonID = xmlUtil::getAttrValue_string(pNode, "id");
    int platoonSize = xmlUtil::getAttrValue_int(pNode, "size");

    for (rapidxml::xml_node<> *cNode = pNode->first_node(); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != "member")
            throw omnetpp::cRuntimeError("Invalid node name '%s' in element '%s'", cNode->name(), vehicle_platoon_tag.c_str());

        std::vector<std::string> validAttr = {"index", "type"};
        xmlUtil::validityCheck(cNode, validAttr);

        int index = xmlUtil::getAttrValue_int(cNode, "index");
        std::string type_str = xmlUtil::getAttrValue_string(cNode, "type");

        if(index < 0 || index >= platoonSize)
            throw omnetpp::cRuntimeError("attribute 'index' is invalid in element '%s': %d", vehicle_platoon_tag.c_str(), index);

        auto it = allVehiclePlatoon.find(platoonID);
        if(it == allVehiclePlatoon.end())
            throw omnetpp::cRuntimeError("Cannot find platoon '%s' in the platoon map", platoonID.c_str());
        it->second.platoonChild[index] = type_str;
    }
}


void AddNode::addVehiclePlatoon()
{
    for(auto &entry : allVehiclePlatoon)
    {
        if(entry.second.processed)
            continue;

        // wait for the depart time for this platoon
        if(entry.second.depart > omnetpp::simTime().dbl())
            continue;

        std::string platoonID = entry.second.id_str;
        std::string route_str = entry.second.routeID_str;
        std::string from_str = entry.second.from_str;
        std::string to_str = entry.second.to_str;

        std::string vehRouteID = "";
        if(route_str != "") vehRouteID = route_str;
        else if(from_str != "" && to_str != "")
        {
            // append 'from_str', 'via_str' and 'to_str'
            std::vector<std::string> allEdges = {from_str};
            allEdges.insert(allEdges.end(), entry.second.via_str_tokenize.begin(), entry.second.via_str_tokenize.end());
            allEdges.push_back(to_str);

            vehRouteID = getShortestRoute(allEdges);
        }
        else
            throw omnetpp::cRuntimeError("'route' or 'from/to' attributes should be defined in element '%s'", vehicle_platoon_tag.c_str());

        int platoonSize = entry.second.size;

        // lane id that this platoon will be inserted
        auto routeEdges = TraCI->routeGetEdges(vehRouteID);
        std::string laneID = routeEdges[0] + "_" + std::to_string(entry.second.departLane);

        // if no departPos is specified, then find one!
        if(entry.second.departPos == -1)
            entry.second.departPos = getPlatoonStartingPosition(entry.second, platoonSize, laneID);

        // check if we can insert this platoon now
        bool possible = checkPlatoonInsertion(entry.second, platoonSize, laneID);
        if(!possible)
        {
            // print this message only once
            if(entry.second.retryCount == 0)
                LOG_WARNING << boost::format("\n>>> WARNING: Platoon '%s' cannot be inserted at position '%d' of lane '%s' at time '%f', "
                        "because the lane is occupied. Retrying in the next time-steps \n") %
                        platoonID %
                        entry.second.departPos %
                        laneID %
                        omnetpp::simTime().dbl() << std::flush;

            entry.second.retryCount++;
            continue;
        }

        LOG_DEBUG << boost::format("\n>>> AddNode is adding vehicle platoon '%s' ... \n") % platoonID << std::flush;

        RGB colorRGB = Color::colorNameToRGB("blue");
        HSV colorHSV = Color::rgb2hsv(colorRGB.red, colorRGB.green, colorRGB.blue);
        // get different saturation
        std::vector<double> shades = Color::generateColorShades(platoonSize-1);

        std::string vehID = platoonID;
        double departPos = entry.second.departPos;

        // adding platooned vehicles starting from leader
        for(int i = 0; i < platoonSize; i++)
        {
            // follower
            if(i != 0)
            {
                vehID = platoonID + "." + std::to_string(i);
                departPos = -5; // DEPART_POS_LAST
            }

            std::string vehType = entry.second.type_str;

            // do we have a non-homogeneous platoon?
            auto k = entry.second.platoonChild.find(i);
            if(k != entry.second.platoonChild.end())
                vehType = k->second;

            TraCI->vehicleAdd(vehID,
                    vehType,
                    vehRouteID,
                    (int32_t)((omnetpp::simTime().dbl())*1000), // we cannot use entry.second.depart, because it might be in the past
                    departPos,
                    0 /*depart speed*/,
                    entry.second.departLane);

            veh_deferred_attributes_t deferred_entry;

            deferred_entry.plnMode = 2;
            deferred_entry.plnDepth = i;
            deferred_entry.plnId = platoonID;

            // setting platoon size in leader ONLY
            if(i == 0)
                deferred_entry.plnSize = platoonSize;

            if(entry.second.pltMgmtProt)
            {
                deferred_entry.plnMode = 3;

                // setting parameters in leader ONLY
                if(i == 0)
                {
                    deferred_entry.maxSize = entry.second.maxSize;
                    deferred_entry.optSize = entry.second.optSize;
                }

                // gap between platoons
                deferred_entry.interGap = entry.second.interGap;
            }

            addDeferredAttribute(vehID, deferred_entry);

            if(!entry.second.pltMgmtProt)
            {
                // set the leader color
                RGB newColor = Color::colorNameToRGB(entry.second.color_str);
                TraCI->vehicleSetColor(vehID, newColor);
            }
            else
            {
                if(i == 0)
                {
                    // set the leader color to red
                    RGB newColor = Color::colorNameToRGB("red");
                    TraCI->vehicleSetColor(vehID, newColor);
                }
                else
                {
                    HSV newColorHSV = {colorHSV.hue, shades[i-1], colorHSV.value};
                    RGB newColorRGB = Color::hsv2rgb(newColorHSV.hue, newColorHSV.saturation, newColorHSV.value);
                    TraCI->vehicleSetColor(vehID, newColorRGB);
                }
            }

            // disable lane changing
            TraCI->vehicleSetLaneChangeMode(vehID, LANECHANGEMODE_OBSTACLE);

            if(i == 0)
            {
                TraCI->vehicleSetSpeed(vehID, entry.second.platoonMaxSpeed);
            }
            else
            {
                if(entry.second.fastCatchUp)
                {
                    // for some reasons changing the speed mode does not work.
                    // tested with stock SUMO map
                    // as an alternative solution, we change the max acceleration directly

                    // the followers should be able to catch up
                    //double vehMaxSpeed = TraCI->vehicleGetMaxSpeed(vehID);
                    //TraCI->vehicleSetSpeed(vehID, vehMaxSpeed);

                    // disregard maximum acceleration limit
                    //TraCI->vehicleSetSpeedMode(vehID, 0b11101);

                    TraCI->vehicleSetMaxAccel(vehID, 200);
                }
            }
        }

        entry.second.processed = true;
    }
}


double AddNode::getPlatoonStartingPosition(vehiclePlatoonEntry_t &platoonEntry, int platoonSize, std::string laneID)
{
    double vehLength = TraCI->vehicleTypeGetLength(platoonEntry.type_str);
    double minGap = TraCI->vehicleTypeGetMinGap(platoonEntry.type_str);

    double safeStartPos = platoonSize*vehLength + (platoonSize-1)*minGap;

    // get the lane length
    double laneLength = TraCI->laneGetLength(laneID);

    if(safeStartPos > laneLength)
        throw omnetpp::cRuntimeError("Platoon '%s' does not fit on lane '%s'", platoonEntry.id_str.c_str(), laneID.c_str());

    return safeStartPos;
}


bool AddNode::checkPlatoonInsertion(vehiclePlatoonEntry_t &platoonEntry, int platoonSize, std::string laneID)
{
    double vehLength = TraCI->vehicleTypeGetLength(platoonEntry.type_str);
    double minGap = TraCI->vehicleTypeGetMinGap(platoonEntry.type_str);

    // get the length of this platoon
    double platoon_from = platoonEntry.departPos + minGap;
    double platoon_to = platoonEntry.departPos - platoonSize*vehLength - (platoonSize-1)*minGap;

    // get the lane length
    double laneLength = TraCI->laneGetLength(laneID);

    if(platoon_from > laneLength || platoon_to < 0)
        throw omnetpp::cRuntimeError("departPos of platoon '%s' is invalid", platoonEntry.id_str.c_str());

    // get all vehicles that are on this lane at this moment
    auto vehicles = TraCI->laneGetLastStepVehicleIDs(laneID);

    for(auto &veh : vehicles)
    {
        double veh_lanePos = TraCI->vehicleGetLanePosition(veh);
        double veh_minGap = TraCI->vehicleGetMinGap(veh);
        double veh_length = TraCI->vehicleGetLength(veh);

        double veh_from = veh_lanePos + veh_minGap;
        // veh_to can be negative
        double veh_to = veh_lanePos - veh_length;

        // if the platoon space is occupied
        if((platoon_to >= veh_to && platoon_to <= veh_from) ||
                (platoon_from >= veh_to && platoon_from <= veh_from) ||
                (platoon_to <= veh_to && platoon_from >= veh_from))
        {
            return false;
        }
    }

    return true;
}


void AddNode::parseCA(rapidxml::xml_node<> *cNode)
{
    ASSERT(std::string(cNode->name()) == ca_tag);

    std::vector<std::string> validAttr = {"id", "pos"};
    xmlUtil::validityCheck(cNode, validAttr);

    std::string id_str = xmlUtil::getAttrValue_string(cNode, "id");
    TraCICoord pos = xmlUtil::getAttrValue_coord(cNode, "pos");

    auto it = allCA.find(id_str);
    if(it == allCA.end())
    {
        // check if the new node has overlap with any of the existing nodes
        for(auto &entry : allCA)
        {
            if(entry.second.pos == pos)
                LOG_WARNING << boost::format("WARNING: CA '%s' is placed on top of '%s'. \n") % id_str % entry.second.id_str;
        }

        CAEntry_t entry = {};

        entry.id_str = id_str;
        entry.pos = pos;

        allCA.insert(std::make_pair(id_str, entry));
    }
    else
        throw omnetpp::cRuntimeError("Multiple '%s' with the same 'id' %s is not allowed!", ca_tag.c_str(), id_str.c_str());
}


void AddNode::addCA()
{
    if(allCA.empty())
        return;

    unsigned int num = allCA.size();

    LOG_DEBUG << boost::format("\n>>> AddNode is adding %1% CA modules ... \n") % num << std::flush;

    cModule* parentMod = getParentModule();
    if (!parentMod)
        throw omnetpp::cRuntimeError("Parent Module not found");

    omnetpp::cModuleType* nodeType = omnetpp::cModuleType::get(par("CA_ModuleType"));

    int i = 0;
    for(auto &entry : allCA)
    {
        // create an array of adversaries
        cModule* mod = nodeType->create(par("CA_ModuleName"), parentMod, num, i);
        mod->finalizeParameters();
        mod->getDisplayString().parse(par("CA_ModuleDisplayString"));
        mod->buildInside();

        TraCI->addMapping(entry.second.id_str, mod->getFullName());

        Coord co = TraCI->convertCoord_traci2omnet(entry.second.pos);

        mod->getSubmodule("mobility")->par("x") = co.x;
        mod->getSubmodule("mobility")->par("y") = co.y;
        mod->getSubmodule("mobility")->par("z") = co.z;

        mod->scheduleStart(omnetpp::simTime());
        mod->callInitialize();

        // store the cModule
        entry.second.module = mod;

        i++;
    }
}


void AddNode::addCircle(std::string name, std::string type, const RGB color, bool filled, TraCICoord center, double radius)
{
    std::list<TraCICoord> circlePoints;

    // Convert from degrees to radians via multiplication by PI/180
    for(int angleInDegrees = 0; angleInDegrees <= 360; angleInDegrees += 10)
    {
        double x = (double)( radius * cos(angleInDegrees * 3.14 / 180) ) + center.x;
        double y = (double)( radius * sin(angleInDegrees * 3.14 / 180) ) + center.y;

        circlePoints.push_back(TraCICoord(x, y));
    }

    // create polygon in SUMO
    TraCI->polygonAdd(name, type, color, filled /*filled*/, 1 /*layer*/, circlePoints);
}


void AddNode::printLoadedStatistics()
{
    LOG_DEBUG << "\n>>> AddNode is done adding nodes. Here is a summary: \n" << std::flush;

    LOG_DEBUG << boost::format("  Number of shortest route calculation: %d \n") % routeCalculation;

    //###################################
    // Get the list of all possible route
    //###################################

    auto loadedRouteList = TraCI->routeGetIDList();
    LOG_DEBUG << boost::format("  %1% routes are loaded: \n      ") % loadedRouteList.size();
    for(std::string route : loadedRouteList)
        LOG_DEBUG << boost::format("%1%, ") % route;

    LOG_DEBUG << "\n";

    //##################################
    // Get the list of all vehicle types
    //##################################

    auto loadedVehTypeList = TraCI->vehicleTypeGetIDList();
    LOG_DEBUG << boost::format("  %1% vehicle types are loaded: \n      ") % loadedVehTypeList.size();
    for(std::string type : loadedVehTypeList)
        LOG_DEBUG << boost::format("%1%, ") % type;

    LOG_DEBUG << "\n";

    //#############################
    // Get the list of all vehicles
    //#############################

    auto loadedVehList = TraCI->simulationGetLoadedVehiclesIDList();

    for (auto it = loadedVehList.begin(); it != loadedVehList.end();)
    {
        if (std::find(TraCI->removed_vehicles.begin(), TraCI->removed_vehicles.end(), *it ) != TraCI->removed_vehicles.end())
            it = loadedVehList.erase( it );
        else
            ++it;
    }

    LOG_DEBUG << boost::format("  %1% vehicles are loaded: \n") % loadedVehList.size();
    // get vehicle type distribution
    std::list<std::string> loadedVehType;
    for(std::string vehID : loadedVehList)
    {
        std::string type = TraCI->vehicleGetTypeID(vehID);
        loadedVehType.push_back(type);
    }
    std::list<std::string> loadedVehTypeListUnique = loadedVehType;
    loadedVehTypeListUnique.sort();  // we need sort the list first before calling unique
    loadedVehTypeListUnique.unique();
    for(std::string type : loadedVehTypeListUnique)
    {
        int count = std::count(loadedVehType.begin(), loadedVehType.end(), type);
        LOG_DEBUG << boost::format("      %1% nodes are added of type \"%2%\" \n") % count % type;
    }

    LOG_DEBUG << "\n";

    // get route distribution
    std::list<std::string> loadedVehRoute;
    for(std::string vehID : loadedVehList)
    {
        std::string route = TraCI->vehicleGetRouteID(vehID);
        loadedVehRoute.push_back(route);
    }
    std::list<std::string> loadedVehRouteListUnique = loadedVehRoute;
    loadedVehRouteListUnique.sort();  // we need sort the list first before calling unique
    loadedVehRouteListUnique.unique();
    for(std::string route : loadedVehRouteListUnique)
    {
        int count = std::count(loadedVehRoute.begin(), loadedVehRoute.end(), route);
        LOG_DEBUG << boost::format("      %1% nodes have route \"%2%\" \n") % count % route;
    }

    LOG_DEBUG << "\n" << std::flush;
}


AddNode * AddNode::getAddNodeInterface()
{
    // get a pointer to the AddNode module
    omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("addNode");
    // make sure module AddNode exists
    ASSERT(module);

    AddNode *ADDNODE = static_cast<AddNode*>(module);
    ASSERT(ADDNODE);

    return ADDNODE;
}


veh_deferred_attributes_t AddNode::getDeferredAttribute(std::string SUMOID)
{
    auto ii = vehs_deferred_attributes.find(SUMOID);

    if(ii != vehs_deferred_attributes.end())
        return ii->second;
    else
    {
        veh_deferred_attributes_t empty = {};
        return empty;
    }
}


bool AddNode::hasDeferredAttribute(std::string SUMOID)
{
    auto ii = vehs_deferred_attributes.find(SUMOID);

    if(ii != vehs_deferred_attributes.end())
        return true;

    return false;
}


void AddNode::addDeferredAttribute(std::string SUMOID, veh_deferred_attributes_t def)
{
    auto ii = vehs_deferred_attributes.find(SUMOID);
    if(ii != vehs_deferred_attributes.end())
        throw omnetpp::cRuntimeError("Vehicle '%s' is already assigned a deferred attribute.", SUMOID);

    vehs_deferred_attributes[SUMOID] = def;
}


void AddNode::removeDeferredAttribute(std::string SUMOID)
{
    auto ii = vehs_deferred_attributes.find(SUMOID);

    if(ii != vehs_deferred_attributes.end())
        vehs_deferred_attributes.erase(ii);
}


void AddNode::updateDeferredAttribute_ip(std::string SUMOID, std::string ip)
{
    auto ii = vehs_deferred_attributes.find(SUMOID);

    // if the vehicle has no deferred attributes, then
    // create a new entry and update ipv4
    if(ii == vehs_deferred_attributes.end())
    {
        veh_deferred_attributes_t entry = {};
        entry.ipv4 = ip;

        vehs_deferred_attributes[SUMOID] = entry;
    }
    // if the vehicle already has a deferred attribute, then
    // update the ipv4 entry only
    else
        ii->second.ipv4 = ip;
}


void AddNode::updateDeferredAttribute_color(std::string SUMOID, std::string color)
{
    auto ii = vehs_deferred_attributes.find(SUMOID);

    // if the vehicle has no deferred attributes, then
    // create a new entry and update color
    if(ii == vehs_deferred_attributes.end())
    {
        veh_deferred_attributes_t entry = {};
        entry.color = color;

        vehs_deferred_attributes[SUMOID] = entry;
    }
    // if the vehicle already has a deferred attribute, then
    // update the color entry only
    else
        ii->second.color = color;
}


adversaryEntry_t AddNode::addNodeGetAdversary(std::string advID)
{
    auto ii = allAdversary.find(advID);

    if(ii != allAdversary.end())
        return ii->second;
    else
    {
        adversaryEntry_t empty = {};
        return empty;
    }
}


RSUEntry_t AddNode::addNodeGetRSU(std::string RSUID)
{
    auto ii = allRSU.find(RSUID);

    if(ii != allRSU.end())
        return ii->second;
    else
    {
        RSUEntry_t empty = {};
        return empty;
    }
}


obstacleEntry_t AddNode::addNodeGetObstacle(std::string obsID)
{
    auto ii = allObstacle.find(obsID);

    if(ii != allObstacle.end())
        return ii->second;
    else
    {
        obstacleEntry_t empty = {};
        return empty;
    }
}


vehicleEntry_t AddNode::addNodeGetVehicle(std::string vehID)
{
    auto ii = allVehicle.find(vehID);

    if(ii != allVehicle.end())
        return ii->second;
    else
    {
        vehicleEntry_t empty = {};
        return empty;
    }
}


vehicleFlowEntry_t AddNode::addNodeGetVehicleFlow(std::string flowID)
{
    auto ii = allVehicleFlow.find(flowID);

    if(ii != allVehicleFlow.end())
        return ii->second;
    else
    {
        vehicleFlowEntry_t empty = {};
        return empty;
    }
}


vehicleMultiFlowEntry_t AddNode::addNodeGetMultiFlow(std::string multiFlowID)
{
    auto ii = allVehicleMultiFlow.find(multiFlowID);

    if(ii != allVehicleMultiFlow.end())
        return ii->second;
    else
    {
        vehicleMultiFlowEntry_t empty = {};
        return empty;
    }
}


vehiclePlatoonEntry_t AddNode::addNodeGetPlatoon(std::string platoonID)
{
    auto ii = allVehiclePlatoon.find(platoonID);

    if(ii != allVehiclePlatoon.end())
        return ii->second;
    else
    {
        vehiclePlatoonEntry_t empty = {};
        return empty;
    }
}


CAEntry_t AddNode::addNodeGetCA(std::string CAID)
{
    auto ii = allCA.find(CAID);

    if(ii != allCA.end())
        return ii->second;
    else
    {
        CAEntry_t empty = {};
        return empty;
    }
}

}

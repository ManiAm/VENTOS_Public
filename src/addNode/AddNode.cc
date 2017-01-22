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

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>

#include "addNode/AddNode.h"
#include "MIXIM/connectionManager/ConnectionManager.h"
#include "logging/vlog.h"

namespace VENTOS {

Define_Module(VENTOS::AddNode);

AddNode::~AddNode()
{

}


void AddNode::initialize(int stage)
{
    super::initialize(stage);

    if(stage ==0)
    {
        id = par("id").stringValue();
        if(id == "")
            return;

        // get a pointer to the TraCI module
        omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        terminate = module->par("terminate").doubleValue();
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initialize_withTraCI", this);
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
    }

    // delete all CA modules in omnet
    for(auto &i : allCA)
    {
        cModule* mod = i.second.module;
        ASSERT(mod);

        cc->unregisterNic(mod->getSubmodule("nic"));
        mod->callFinish();
        mod->deleteModule();
    }

    // delete all RSU modules in omnet
    for(auto &i : allRSU)
    {
        cModule* mod = i.second.module;
        ASSERT(mod);

        cc->unregisterNic(mod->getSubmodule("nic"));
        mod->callFinish();
        mod->deleteModule();
    }

    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("initialize_withTraCI", this);
}


void AddNode::handleMessage(omnetpp::cMessage *msg)
{
    if(msg->isSelfMessage() && msg->getKind() == TYPE_TIMER)
    {
        // remove the obstacle vehicle from SUMO
        std::string vehID = msg->getName();
        TraCI->vehicleRemove(vehID, 2);
    }
}


void AddNode::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        boost::filesystem::path sumoFull = TraCI->getFullPath_SUMOConfig();
        boost::filesystem::path sumoDir = sumoFull.parent_path();
        boost::filesystem::path addNodePath = sumoDir / "traci_addNode.xml";

        if ( !boost::filesystem::exists(addNodePath) )
            throw omnetpp::cRuntimeError("File '%s' does not exist!", addNodePath.c_str());

        SUMO_timeStep = TraCI->simulationGetTimeStep();

        readInsertion(addNodePath.string());
    }
}


void AddNode::readInsertion(std::string addNodePath)
{
    rapidxml::file<> xmlFile(addNodePath.c_str());  // Convert our file to a rapid-xml readable object
    rapidxml::xml_document<> doc;                   // Build a rapidxml doc
    doc.parse<0>(xmlFile.data());                   // Fill it with data from our file

    // Get the first applDependency node
    rapidxml::xml_node<> *pNode = doc.first_node("addNode");

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
                throw omnetpp::cRuntimeError("Cannot find id '%s' in the traci_addNode.xml file!", this->id.c_str());
        }
    }

    // format checking: Iterate over all nodes in this id
    for(rapidxml::xml_node<> *cNode = pNode->first_node(); cNode; cNode = cNode->next_sibling())
    {
        std::string nodeName = cNode->name();

        if(nodeName != adversary_tag &&
                nodeName != ca_tag &&
                nodeName != rsu_tag &&
                nodeName != obstacle_tag &&
                nodeName != vehicle_tag &&
                nodeName != vehicle_flow_tag &&
                nodeName != emulated_tag)
            throw omnetpp::cRuntimeError("'%s' is not a valid node in id '%s'", this->id.c_str());
    }

    parseAdversary(pNode);
    parseCA(pNode);
    parseRSU(pNode);
    parseObstacle(pNode);
    parseVehicle(pNode);
    parseVehicleFlow(pNode);
    parseEmulated(pNode);

    if(allAdversary.empty() &&
            allCA.empty() &&
            allRSU.empty() &&
            allObstacle.empty() &&
            allVehicle.empty() &&
            allVehicleFlow.empty() &&
            allEmulated.empty())
        LOG_WARNING << boost::format("\nWARNING: Add node with id '%1%' is empty! \n") % this->id << std::flush;

    addAdversary();
    addCA();
    addRSU();
    addObstacle();
    addVehicle();
    addVehicleFlow();
    addEmulated(); // should be called last!

    if(!allVehicle.empty() || !allVehicleFlow.empty())
    {
        if(LOG_ACTIVE(DEBUG_LOG_VAL))
            printLoadedStatistics();
    }
}



void AddNode::parseAdversary(rapidxml::xml_node<> *pNode)
{
    // Iterate over all 'adversary' nodes
    for(rapidxml::xml_node<> *cNode = pNode->first_node(adversary_tag.c_str()); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != adversary_tag)
            continue;

        // format checking: Iterate over all attributes in this node
        for(rapidxml::xml_attribute<> *cAttr1 = cNode->first_attribute(); cAttr1; cAttr1 = cAttr1->next_attribute())
        {
            std::string attName = cAttr1->name();

            if(attName != "id" && attName != "pos")
                throw omnetpp::cRuntimeError("'%s' is not a valid attribute in node '%s'", attName.c_str(), adversary_tag.c_str());
        }

        rapidxml::xml_attribute<> *cAttr = cNode->first_attribute("id");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'id' attribute is not found in %s node", adversary_tag.c_str());
        std::string id_str = cAttr->value();
        boost::trim(id_str);

        cAttr = cNode->first_attribute("pos");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'pos' attribute is not found in %s node", adversary_tag.c_str());
        std::string pos_str = cAttr->value();
        boost::trim(pos_str);

        // pos_str are separated by ,
        std::vector<std::string> pos;
        boost::split(pos, pos_str, boost::is_any_of(","));

        if(pos.size() != 3)
            throw omnetpp::cRuntimeError("'pos' attribute in %s node should be in the \"x,y,z\" format", adversary_tag.c_str());

        double pos_x = 0, pos_y = 0, pos_z = 0;

        try
        {
            std::string pos_x_str = pos[0];
            boost::trim(pos_x_str);
            pos_x = boost::lexical_cast<double>(pos_x_str);

            std::string pos_y_str = pos[1];
            boost::trim(pos_y_str);
            pos_y = boost::lexical_cast<double>(pos_y_str);

            std::string pos_z_str = pos[2];
            boost::trim(pos_z_str);
            pos_z = boost::lexical_cast<double>(pos_z_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'pos' attribute is badly formatted in %s node: %s", pos_str.c_str(), adversary_tag.c_str());
        }

        auto it = allAdversary.find(id_str);
        if(it == allAdversary.end())
        {
            // check if the new node has overlap with any of the existing nodes
            for(auto &entry : allAdversary)
            {
                if(entry.second.pos_x == pos_x && entry.second.pos_y == pos_y && entry.second.pos_z == pos_z)
                    LOG_WARNING << boost::format("WARNING: Adversary '%s' is placed on top of '%s'. \n") % id_str % entry.second.id_str;
            }

            adversaryEntry_t entry = {id_str, pos_x, pos_y, pos_z};
            allAdversary.insert(std::make_pair(id_str, entry));
        }
        else
            throw omnetpp::cRuntimeError("Multiple %s with the same 'id' %s is not allowed!", adversary_tag.c_str(), id_str.c_str());
    }
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

        mod->getSubmodule("mobility")->par("x") = entry.second.pos_x;
        mod->getSubmodule("mobility")->par("y") = entry.second.pos_y;
        mod->getSubmodule("mobility")->par("z") = entry.second.pos_z;

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

        Coord *center = new Coord(entry.second.pos_x, entry.second.pos_y);
        addCircle(entry.second.id_str, par("adversary_ModuleName"), Color::colorNameToRGB("green"), 1, center, radius);

        i++;
    }
}


void AddNode::parseCA(rapidxml::xml_node<> *pNode)
{
    // Iterate over all 'CA' nodes
    for(rapidxml::xml_node<> *cNode = pNode->first_node(ca_tag.c_str()); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != ca_tag)
            continue;

        // format checking: Iterate over all attributes in this node
        for(rapidxml::xml_attribute<> *cAttr1 = cNode->first_attribute(); cAttr1; cAttr1 = cAttr1->next_attribute())
        {
            std::string attName = cAttr1->name();

            if(attName != "id" && attName != "pos")
                throw omnetpp::cRuntimeError("'%s' is not a valid attribute in node '%s'", attName.c_str(), ca_tag.c_str());
        }

        rapidxml::xml_attribute<> *cAttr = cNode->first_attribute("id");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'id' attribute is not found in %s node", ca_tag.c_str());
        std::string id_str = cAttr->value();
        boost::trim(id_str);

        cAttr = cNode->first_attribute("pos");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'pos' attribute is not found in %s node", ca_tag.c_str());
        std::string pos_str = cAttr->value();
        boost::trim(pos_str);

        // pos_str are separated by ,
        std::vector<std::string> pos;
        boost::split(pos, pos_str, boost::is_any_of(","));

        if(pos.size() != 3)
            throw omnetpp::cRuntimeError("'pos' attribute in %s node should be in the \"x,y,z\" format.", ca_tag.c_str());

        double pos_x = 0, pos_y = 0, pos_z = 0;

        try
        {
            std::string pos_x_str = pos[0];
            boost::trim(pos_x_str);
            pos_x = boost::lexical_cast<double>(pos_x_str);

            std::string pos_y_str = pos[1];
            boost::trim(pos_y_str);
            pos_y = boost::lexical_cast<double>(pos_y_str);

            std::string pos_z_str = pos[2];
            boost::trim(pos_z_str);
            pos_z = boost::lexical_cast<double>(pos_z_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'pos' attribute is badly formatted in %s node: %s", ca_tag.c_str(), pos_str.c_str());
        }

        auto it = allCA.find(id_str);
        if(it == allCA.end())
        {
            // check if the new node has overlap with any of the existing nodes
            for(auto &entry : allCA)
            {
                if(entry.second.pos_x == pos_x && entry.second.pos_y == pos_y && entry.second.pos_z == pos_z)
                    LOG_WARNING << boost::format("WARNING: CA '%s' is placed on top of '%s'. \n") % id_str % entry.second.id_str;
            }

            CAEntry_t entry = {id_str, pos_x, pos_y, pos_z};
            allCA.insert(std::make_pair(id_str, entry));
        }
        else
            throw omnetpp::cRuntimeError("Multiple %s with the same 'id' %s is not allowed!", ca_tag.c_str(), id_str.c_str());
    }
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
        mod->getDisplayString().parse(par("adversary_ModuleDisplayString"));
        mod->buildInside();

        mod->getSubmodule("mobility")->par("x") = entry.second.pos_x;
        mod->getSubmodule("mobility")->par("y") = entry.second.pos_y;
        mod->getSubmodule("mobility")->par("z") = entry.second.pos_z;

        mod->scheduleStart(omnetpp::simTime());
        mod->callInitialize();

        // store the cModule
        entry.second.module = mod;

        i++;
    }
}


void AddNode::parseRSU(rapidxml::xml_node<> *pNode)
{
    // Iterate over all 'RSU' nodes
    for(rapidxml::xml_node<> *cNode = pNode->first_node(rsu_tag.c_str()); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != rsu_tag)
            continue;

        // format checking: Iterate over all attributes in this node
        for(rapidxml::xml_attribute<> *cAttr1 = cNode->first_attribute(); cAttr1; cAttr1 = cAttr1->next_attribute())
        {
            std::string attName = cAttr1->name();

            if(attName != "id" && attName != "pos" && attName != "drawMaxIntfDist" && attName != "color" && attName != "filled")
                throw omnetpp::cRuntimeError("'%s' is not a valid attribute in node '%s'", attName.c_str(), rsu_tag.c_str());
        }

        rapidxml::xml_attribute<> *cAttr = cNode->first_attribute("id");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'id' attribute is not found in %s node", rsu_tag.c_str());
        std::string id_str = cAttr->value();
        boost::trim(id_str);

        cAttr = cNode->first_attribute("pos");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'pos' attribute is not found in %s node", rsu_tag.c_str());
        std::string pos_str = cAttr->value();
        boost::trim(pos_str);

        // pos_str are separated by ,
        std::vector<std::string> pos;
        boost::split(pos, pos_str, boost::is_any_of(","));

        if(pos.size() != 3)
            throw omnetpp::cRuntimeError("'pos' attribute in %s node should be in the \"x,y,z\" format", rsu_tag.c_str());

        double pos_x = 0, pos_y = 0, pos_z = 0;

        try
        {
            std::string pos_x_str = pos[0];
            boost::trim(pos_x_str);
            pos_x = boost::lexical_cast<double>(pos_x_str);

            std::string pos_y_str = pos[1];
            boost::trim(pos_y_str);
            pos_y = boost::lexical_cast<double>(pos_y_str);

            std::string pos_z_str = pos[2];
            boost::trim(pos_z_str);
            pos_z = boost::lexical_cast<double>(pos_z_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'pos' attribute is badly formatted in %s node: %s", rsu_tag.c_str(), pos_str.c_str());
        }

        bool drawMaxIntfDist = true;
        cAttr = cNode->first_attribute("drawMaxIntfDist");
        if(cAttr)
        {
            std::string drawMaxIntfDist_str = cAttr->value();
            boost::trim(drawMaxIntfDist_str);

            if(drawMaxIntfDist_str == "true")
                drawMaxIntfDist = true;
            else if(drawMaxIntfDist_str == "false")
                drawMaxIntfDist = false;
            else
                throw omnetpp::cRuntimeError("'drawMaxIntfDist' attribute is badly formatted in %s node: %s", rsu_tag.c_str(), drawMaxIntfDist_str.c_str());
        }

        std::string color_str = "green";
        cAttr = cNode->first_attribute("color");
        if(cAttr)
        {
            color_str = cAttr->value();
            boost::trim(color_str);
        }

        bool filled = false;
        cAttr = cNode->first_attribute("filled");
        if(cAttr)
        {
            std::string filled_str = cAttr->value();
            boost::trim(filled_str);

            if(filled_str == "true")
                filled = true;
            else if(filled_str == "false")
                filled = false;
            else
                throw omnetpp::cRuntimeError("'filled' attribute is badly formatted in %s node: %s", rsu_tag.c_str(), filled_str.c_str());
        }

        auto it = allRSU.find(id_str);
        if(it == allRSU.end())
        {
            // check if the new node has overlap with any of the existing nodes
            for(auto &entry : allRSU)
            {
                if(entry.second.pos_x == pos_x && entry.second.pos_y == pos_y && entry.second.pos_z == pos_z)
                    LOG_WARNING << boost::format("WARNING: RSU '%s' is placed on top of '%s'. \n") % id_str % entry.second.id_str;
            }

            RSUEntry_t entry = {};
            entry.id_str = id_str;
            entry.pos_x = pos_x;
            entry.pos_y = pos_y;
            entry.pos_z = pos_z;
            entry.drawMaxIntfDist = drawMaxIntfDist;
            entry.color_str = color_str;
            entry.filled = filled;

            allRSU.insert(std::make_pair(id_str, entry));
        }
        else
            throw omnetpp::cRuntimeError("Multiple %s with the same 'id' %s is not allowed!", rsu_tag.c_str(), id_str.c_str());
    }
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

    // get all junctions in the network
    auto junctionList = TraCI->junctionGetIDList();

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

        double rsu_x = entry.second.pos_x;
        double rsu_y = entry.second.pos_y;

        // the RSU is associated with a TL
        if(myTLid != "")
        {
            // look for the junction with the same name
            auto junc = std::find(junctionList.begin(), junctionList.end(), myTLid);

            if(junc != junctionList.end())
            {
                // calculate the distance from this RSU to the center of the junction
                auto coord = TraCI->junctionGetPosition(myTLid);
                double dist = sqrt(pow(rsu_x - coord.x, 2.) + pow(rsu_y - coord.y, 2.));

                if(dist > 100)
                    LOG_WARNING << boost::format("\nWARNING: RSU '%s' is not aligned with the intersection. \n") % entry.second.id_str;
            }
        }
        else
        {
            double shortest_dist = std::numeric_limits<int>::max();
            std::string nearest_jun = "";

            // iterate over junctions
            for(auto &junc : junctionList)
            {
                // skip the internal junctions
                if(junc[0] == ':')
                    continue;

                // calculate the distance from this RSU to the center of the junction
                auto coord = TraCI->junctionGetPosition(junc);
                double dist = sqrt(pow(rsu_x - coord.x, 2.) + pow(rsu_y - coord.y, 2.));

                if(dist < shortest_dist)
                {
                    shortest_dist = dist;
                    nearest_jun = junc;
                }
            }

            if(shortest_dist < 100)
                LOG_WARNING << boost::format("\nWARNING: RSU '%s' is in the vicinity of intersection '%s'. Did you forget to associate? \n") % entry.second.id_str % nearest_jun;
        }

        // create an array of RSUs
        cModule* mod = nodeType->create(par("RSU_ModuleName"), parentMod, num, i);
        mod->finalizeParameters();
        mod->getDisplayString().parse(par("RSU_ModuleDisplayString"));
        mod->buildInside();

        mod->getSubmodule("mobility")->par("x") = entry.second.pos_x;
        mod->getSubmodule("mobility")->par("y") = entry.second.pos_y;
        mod->getSubmodule("mobility")->par("z") = entry.second.pos_z;

        mod->getSubmodule("appl")->par("myTLid") = myTLid;
        mod->getSubmodule("appl")->par("SUMOID") = entry.second.id_str;

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
        {
            Coord *center = new Coord(entry.second.pos_x, entry.second.pos_y);
            addCircle(entry.second.id_str, par("RSU_ModuleName"), Color::colorNameToRGB(entry.second.color_str), entry.second.filled, center, radius);
        }
    }
}


void AddNode::parseObstacle(rapidxml::xml_node<> *pNode)
{
    // Iterate over all 'obstacle' nodes
    for(rapidxml::xml_node<> *cNode = pNode->first_node(obstacle_tag.c_str()); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != obstacle_tag)
            continue;

        // format checking: Iterate over all attributes in this node
        for(rapidxml::xml_attribute<> *cAttr1 = cNode->first_attribute(); cAttr1; cAttr1 = cAttr1->next_attribute())
        {
            std::string attName = cAttr1->name();

            if(attName != "id" &&
                    attName != "length" &&
                    attName != "edge" &&
                    attName != "lane" &&
                    attName != "lanePos" &&
                    attName != "onRoad" &&
                    attName != "color" &&
                    attName != "begin" &&
                    attName != "end" &&
                    attName != "duration")
                throw omnetpp::cRuntimeError("'%s' is not a valid attribute in node '%s'", attName.c_str(), obstacle_tag.c_str());
        }

        rapidxml::xml_attribute<> *cAttr = cNode->first_attribute("id");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'id' attribute is not found in %s node", obstacle_tag.c_str());
        std::string id_str = cAttr->value();

        std::string length_str = "5";
        cAttr = cNode->first_attribute("length");
        if(cAttr)
        {
            length_str = cAttr->value();
            boost::trim(length_str);
        }

        int length = 0;
        try
        {
            length = boost::lexical_cast<int>(length_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'length' attribute is badly formatted in %s node: %s", obstacle_tag.c_str(), length_str.c_str());
        }

        cAttr = cNode->first_attribute("edge");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'edge' attribute is not found in %s node", obstacle_tag.c_str());
        std::string edge_str = cAttr->value();
        boost::trim(edge_str);

        cAttr = cNode->first_attribute("lane");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'lane' attribute is not found in %s node", obstacle_tag.c_str());
        std::string lane_str = cAttr->value();
        boost::trim(lane_str);

        int lane = 0;
        try
        {
            lane = boost::lexical_cast<int>(lane_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'lane' attribute is badly formatted in %s node: %s", obstacle_tag.c_str(), lane_str.c_str());
        }

        cAttr = cNode->first_attribute("lanePos");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'lanePos' attribute is not found in %s node", obstacle_tag.c_str());
        std::string lanePos_str = cAttr->value();
        boost::trim(lanePos_str);

        double lanePos = 0;
        try
        {
            lanePos = boost::lexical_cast<double>(lanePos_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'lanePos' attribute is badly formatted in %s node: %s", obstacle_tag.c_str(), lanePos_str.c_str());
        }

        bool onRoad = true;
        cAttr = cNode->first_attribute("onRoad");
        if(cAttr)
        {
            std::string onRoad_str = cAttr->value();
            boost::trim(onRoad_str);

            if(onRoad_str == "true")
                onRoad = true;
            else if(onRoad_str == "false")
                onRoad = false;
            else
                throw omnetpp::cRuntimeError("'onRoad' attribute is badly formatted in %s node: %s", obstacle_tag.c_str(), onRoad_str.c_str());
        }

        std::string color_str = "red";
        cAttr = cNode->first_attribute("color");
        if(cAttr)
        {
            color_str = cAttr->value();
            boost::trim(color_str);
        }

        std::string begin_str = "0";
        cAttr = cNode->first_attribute("begin");
        if(cAttr)
        {
            begin_str = cAttr->value();
            boost::trim(begin_str);
        }

        double begin = 0;
        try
        {
            begin = boost::lexical_cast<double>(begin_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'begin' attribute is badly formatted in %s node: %s", obstacle_tag.c_str(), begin_str.c_str());
        }

        double end = -1;
        double duration = -1;

        if((cAttr = cNode->first_attribute("end")))
        {
            std::string end_str = cAttr->value();
            boost::trim(end_str);

            try
            {
                end = boost::lexical_cast<double>(end_str);
            }
            catch (boost::bad_lexical_cast const&)
            {
                throw omnetpp::cRuntimeError("'end' attribute is badly formatted in %s node: %s", obstacle_tag.c_str(), end_str.c_str());
            }

            if(end <= begin)
                throw omnetpp::cRuntimeError("'end' value is less than 'begin' in %s node: %s", obstacle_tag.c_str(), end_str.c_str());

            cAttr = cNode->first_attribute("duration");
            if(cAttr)
                throw omnetpp::cRuntimeError("'duration' attribute is redundant when 'end' is present in %s node", obstacle_tag.c_str());
        }
        else if((cAttr = cNode->first_attribute("duration")))
        {
            std::string duration_str = cAttr->value();
            boost::trim(duration_str);

            try
            {
                duration = boost::lexical_cast<double>(duration_str);
            }
            catch (boost::bad_lexical_cast const&)
            {
                throw omnetpp::cRuntimeError("'duration' attribute is badly formatted in %s node: %s", obstacle_tag.c_str(), duration_str.c_str());
            }

            if(duration <= 0)
                throw omnetpp::cRuntimeError("'duration' value should be positive in %s node: %s", obstacle_tag.c_str(), duration_str.c_str());

            cAttr = cNode->first_attribute("end");
            if(cAttr)
                throw omnetpp::cRuntimeError("'end' attribute is redundant when 'duration' is present in %s node", obstacle_tag.c_str());
        }

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
            throw omnetpp::cRuntimeError("Multiple %s with the same 'id' %s is not allowed!", obstacle_tag.c_str(), id_str.c_str());
    }
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

        // now we add a vehicle as obstacle
        TraCI->vehicleAdd(vehID, "DEFAULT_VEHTYPE", route_name, (int32_t)(entry.second.begin * 1000), entry.second.lanePos, 0, entry.second.lane);

        // and make it stop on the lane!
        TraCI->vehicleSetSpeed(vehID, 0.);
        TraCI->vehicleSetLaneChangeMode(vehID, LANECHANGEMODE_OBSTACLE);

        // and change its color
        RGB newColor = Color::colorNameToRGB(entry.second.color_str);
        TraCI->vehicleSetColor(vehID, newColor);

        // change veh class to "custome1"
        TraCI->vehicleSetClass(vehID, "custom1");

        // change veh length
        TraCI->vehicleSetLength(vehID, entry.second.length);

        // change veh shape
        // todo: update to new version of sumo and set shape

        if(entry.second.end != -1)
        {
            omnetpp::cMessage* obstacleRemobalEvt = new omnetpp::cMessage(vehID.c_str(), TYPE_TIMER);
            scheduleAt(omnetpp::simTime() + entry.second.end, obstacleRemobalEvt);
        }
        else if(entry.second.duration != -1)
        {
            omnetpp::cMessage* obstacleRemobalEvt = new omnetpp::cMessage(vehID.c_str(), TYPE_TIMER);
            scheduleAt(omnetpp::simTime() + entry.second.begin + entry.second.duration, obstacleRemobalEvt);
        }
    }
}


void AddNode::parseVehicle(rapidxml::xml_node<> *pNode)
{
    // Iterate over all 'vehicle' nodes
    for(rapidxml::xml_node<> *cNode = pNode->first_node(vehicle_tag.c_str()); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != vehicle_tag)
            continue;

        // format checking: Iterate over all attributes in this node
        for(rapidxml::xml_attribute<> *cAttr1 = cNode->first_attribute(); cAttr1; cAttr1 = cAttr1->next_attribute())
        {
            std::string attName = cAttr1->name();

            if(attName != "id" &&
                    attName != "type" &&
                    attName != "route" &&
                    attName != "from" &&
                    attName != "to" &&
                    attName != "via" &&
                    attName != "color" &&
                    attName != "status" &&
                    attName != "duration" &&
                    attName != "depart" &&
                    attName != "departSpeed" &&
                    attName != "departPos" &&
                    attName != "departLane" &&
                    attName != "laneChangeMode")
                throw omnetpp::cRuntimeError("'%s' is not a valid attribute in node '%s'", attName.c_str(), vehicle_tag.c_str());
        }

        rapidxml::xml_attribute<> *cAttr = cNode->first_attribute("id");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'id' attribute is not found in %s node", vehicle_tag.c_str());
        std::string id_str = cAttr->value();
        boost::trim(id_str);

        cAttr = cNode->first_attribute("type");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'type' attribute is not found in %s node", vehicle_tag.c_str());
        std::string type_str = cAttr->value();
        boost::trim(type_str);

        std::string routeID_str = "";
        std::string from_str = "";
        std::string to_str = "";

        if((cAttr = cNode->first_attribute("route")))
        {
            routeID_str = cAttr->value();
            boost::trim(routeID_str);

            cAttr = cNode->first_attribute("from");
            if(cAttr)
                throw omnetpp::cRuntimeError("'from' attribute is redundant when 'route' is present in %s node", vehicle_tag.c_str());

            cAttr = cNode->first_attribute("to");
            if(cAttr)
                throw omnetpp::cRuntimeError("'to' attribute is redundant when 'route' is present in %s node", vehicle_tag.c_str());
        }
        else if((cAttr = cNode->first_attribute("from")))
        {
            from_str = cAttr->value();
            boost::trim(from_str);

            cAttr = cNode->first_attribute("to");
            if(!cAttr)
                throw omnetpp::cRuntimeError("'to' attribute does not exist in %s node", vehicle_tag.c_str());

            to_str = cAttr->value();
            boost::trim(to_str);

            cAttr = cNode->first_attribute("route");
            if(cAttr)
                throw omnetpp::cRuntimeError("'route' attribute is redundant when 'from/to' is present in %s node", vehicle_tag.c_str());
        }
        else if((cAttr = cNode->first_attribute("to")))
        {
            to_str = cAttr->value();
            boost::trim(to_str);

            cAttr = cNode->first_attribute("from");
            if(!cAttr)
                throw omnetpp::cRuntimeError("'from' attribute does not exist in %s node", vehicle_tag.c_str());

            from_str = cAttr->value();
            boost::trim(from_str);

            cAttr = cNode->first_attribute("route");
            if(cAttr)
                throw omnetpp::cRuntimeError("'route' attribute is redundant when 'from/to' is present in %s node", vehicle_tag.c_str());
        }
        else
            throw omnetpp::cRuntimeError("either 'route' or 'from/to' attributes should be defined in %s node", vehicle_tag.c_str());

        std::string via_str = "";
        std::vector<std::string> via_str_tokenize;
        cAttr = cNode->first_attribute("via");
        if(cAttr)
        {
            std::string via_str = cAttr->value();
            boost::trim(via_str);

            // tokenize via_str
            boost::split(via_str_tokenize, via_str, boost::is_any_of(","));

            // remove leading/trailing space
            for(auto &entry : via_str_tokenize)
            {
                boost::trim(entry);
                if(entry == "")
                    throw omnetpp::cRuntimeError("'via' attribute is not formatted correctly in %s node", vehicle_tag.c_str());
            }
        }

        std::string color_str = "yellow";
        cAttr = cNode->first_attribute("color");
        if(cAttr)
        {
            color_str = cAttr->value();
            boost::trim(color_str);
        }

        std::string status_str = "";
        cAttr = cNode->first_attribute("status");
        if(cAttr)
        {
            status_str = cAttr->value();
            boost::trim(status_str);

            if(status_str != "stopped" && status_str != "parked")
                throw omnetpp::cRuntimeError("'status' attribute is unknown in %s node: %s", vehicle_tag.c_str(), status_str.c_str());
        }

        double duration = -1;
        cAttr = cNode->first_attribute("duration");
        if(cAttr)
        {
            std::string duration_str = cAttr->value();
            boost::trim(duration_str);

            try
            {
                duration = boost::lexical_cast<double>(duration_str);
            }
            catch (boost::bad_lexical_cast const&)
            {
                throw omnetpp::cRuntimeError("'duration' attribute is badly formatted in %s node: %s", vehicle_tag.c_str(), duration_str.c_str());
            }

            cAttr = cNode->first_attribute("status");
            if(!cAttr)
                throw omnetpp::cRuntimeError("'status' attribute is required when 'duration' is present in %s node", vehicle_tag.c_str());
        }

        std::string depart_str = "0";
        cAttr = cNode->first_attribute("depart");
        if(cAttr)
        {
            depart_str = cAttr->value();
            boost::trim(depart_str);
        }

        double depart = 0;
        try
        {
            depart = boost::lexical_cast<double>(depart_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'depart' attribute is badly formatted in %s node: %s", vehicle_tag.c_str(), depart_str.c_str());
        }

        if(depart < 0)
            throw omnetpp::cRuntimeError("'depart' value is negative in %s node: %s", vehicle_tag.c_str(), depart_str.c_str());

        std::string departSpeed_str = "0";
        cAttr = cNode->first_attribute("departSpeed");
        if(cAttr)
        {
            departSpeed_str = cAttr->value();
            boost::trim(departSpeed_str);
        }

        double departSpeed = 0;
        try
        {
            departSpeed = boost::lexical_cast<double>(departSpeed_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'departSpeed' attribute is badly formatted in %s node: %s", vehicle_tag.c_str(), departSpeed_str.c_str());
        }

        std::string departPos_str = "0";
        cAttr = cNode->first_attribute("departPos");
        if(cAttr)
        {
            departPos_str = cAttr->value();
            boost::trim(departPos_str);
        }

        double departPos = 0;
        try
        {
            departPos = boost::lexical_cast<double>(departPos_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'departPos' attribute is badly formatted in %s node: %s", vehicle_tag.c_str(), departPos_str.c_str());
        }

        std::string departLane_str = "-5"; // DEPART_LANE_BEST_FREE
        cAttr = cNode->first_attribute("departLane");
        if(cAttr)
        {
            departLane_str = cAttr->value();
            boost::trim(departLane_str);
        }

        int departLane = 0;
        try
        {
            departLane = boost::lexical_cast<int>(departLane_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'departLane' attribute is badly formatted in %s node: %s", vehicle_flow_tag.c_str(), departLane_str.c_str());
        }

        std::string laneChangeMode_str = std::to_string(LANECHANGEMODE_DEFAULT);
        cAttr = cNode->first_attribute("laneChangeMode");
        if(cAttr)
        {
            laneChangeMode_str = cAttr->value();
            boost::trim(laneChangeMode_str);
        }

        int laneChangeMode = 0;
        try
        {
            laneChangeMode = boost::lexical_cast<int>(laneChangeMode_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'laneChangeMode' attribute is badly formatted in %s node: %s", vehicle_flow_tag.c_str(), laneChangeMode_str.c_str());
        }

        if(laneChangeMode < 0)
            throw omnetpp::cRuntimeError("'laneChangeMode' value is not valid in %s node: %s", vehicle_flow_tag.c_str(), laneChangeMode_str.c_str());

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
            entry.status_str = status_str;
            entry.duration = duration;
            entry.depart = depart;
            entry.departSpeed = departSpeed;
            entry.departPos = departPos;
            entry.departLane = departLane;
            entry.laneChangeMode = laneChangeMode;

            allVehicle.insert(std::make_pair(id_str, entry));
        }
        else
            throw omnetpp::cRuntimeError("Multiple %s with the same 'id' %s is not allowed!", vehicle_tag.c_str(), id_str.c_str());
    }
}


void AddNode::addVehicle()
{
    if(allVehicle.empty())
        return;

    unsigned int num = allVehicle.size();

    LOG_DEBUG << boost::format("\n>>> AddNode is adding %1% vehicle modules ... \n") % num << std::flush;

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
            throw omnetpp::cRuntimeError("'route' or 'from/to' attributes should be defined in node '%s'", vehicle_tag.c_str());

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
        }
        else if(entry.second.status_str == "parked")
        {
            // get all edges for this route
            std::vector<std::string> edges = TraCI->routeGetEdges(vehRouteID);

            TraCI->vehicleSetStop(vehID, edges[0], entry.second.departPos, entry.second.departLane, entry.second.duration, 1 /*parking*/);
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


void AddNode::parseVehicleFlow(rapidxml::xml_node<> *pNode)
{
    // Iterate over all 'vehicle_flow' nodes
    for(rapidxml::xml_node<> *cNode = pNode->first_node(vehicle_flow_tag.c_str()); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != vehicle_flow_tag)
            continue;

        // format checking: Iterate over all attributes in this node
        for(rapidxml::xml_attribute<> *cAttr1 = cNode->first_attribute(); cAttr1; cAttr1 = cAttr1->next_attribute())
        {
            std::string attName = cAttr1->name();

            if(attName != "id" &&
                    attName != "type" &&
                    attName != "typeDist" &&
                    attName != "color" &&
                    attName != "route" &&
                    attName != "routeDist" &&
                    attName != "speed" &&
                    attName != "lane" &&
                    attName != "lanePos" &&
                    attName != "laneChangeMode" &&
                    attName != "number" &&
                    attName != "begin" &&
                    attName != "end" &&
                    attName != "distribution" &&
                    attName != "period" &&
                    attName != "lambda" &&
                    attName != "seed" &&
                    attName != "probability")
                throw omnetpp::cRuntimeError("'%s' is not a valid attribute in node '%s'", attName.c_str(), vehicle_flow_tag.c_str());
        }

        rapidxml::xml_attribute<> *cAttr = cNode->first_attribute("id");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'id' attribute is not found in %s node", vehicle_flow_tag.c_str());
        std::string id_str = cAttr->value();
        boost::trim(id_str);

        cAttr = cNode->first_attribute("type");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'type' attribute is not found in %s node", vehicle_flow_tag.c_str());
        std::string type_str = cAttr->value();
        boost::trim(type_str);

        // tokenize type
        std::vector<std::string> type_str_tokenize;
        boost::split(type_str_tokenize, type_str, boost::is_any_of(","));

        // remove leading/trailing space
        for(auto &entry : type_str_tokenize)
        {
            boost::trim(entry);
            if(entry == "")
                throw omnetpp::cRuntimeError("'type' attribute is not formatted correctly in %s node", vehicle_flow_tag.c_str());
        }

        std::string typeDist_str = "";
        std::vector<double> typeDist_tokenize;

        // we have multiple types
        if(type_str_tokenize.size() > 1)
        {
            cAttr = cNode->first_attribute("typeDist");
            if(!cAttr)
                throw omnetpp::cRuntimeError("'typeDist' attribute is not found in %s node", vehicle_flow_tag.c_str());
            typeDist_str = cAttr->value();
            boost::trim(typeDist_str);

            // tokenize typeDist_str
            std::vector<std::string> typeDist_str_tokenize;
            boost::split(typeDist_str_tokenize, typeDist_str, boost::is_any_of(","));

            if(type_str_tokenize.size() != typeDist_str_tokenize.size())
                throw omnetpp::cRuntimeError("'type' and 'typeDist' attributes do not match in %s node", vehicle_flow_tag.c_str());

            double sum = 0;
            for(auto &entry : typeDist_str_tokenize)
            {
                boost::trim(entry);

                double typeDist_e = 0;
                try
                {
                    typeDist_e = boost::lexical_cast<double>(entry);
                }
                catch (boost::bad_lexical_cast const&)
                {
                    throw omnetpp::cRuntimeError("'typeDist' attribute is badly formatted in %s node: %s", vehicle_flow_tag.c_str(), typeDist_str.c_str());
                }

                typeDist_tokenize.push_back(typeDist_e);
                sum += typeDist_e;
            }

            if(sum != 100)
                throw omnetpp::cRuntimeError("'typeDist' values do not add up to 100 percent in %s node: %s", vehicle_flow_tag.c_str(), typeDist_str.c_str());
        }

        std::string color_str = "yellow";
        cAttr = cNode->first_attribute("color");
        if(cAttr)
        {
            color_str = cAttr->value();
            boost::trim(color_str);
        }

        cAttr = cNode->first_attribute("route");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'route' attribute is not found in %s node", vehicle_flow_tag.c_str());
        std::string routeID_str = cAttr->value();
        boost::trim(routeID_str);

        // tokenize routeID
        std::vector<std::string> routeID_str_tokenize;
        boost::split(routeID_str_tokenize, routeID_str, boost::is_any_of(","));

        // remove leading/trailing space
        for(auto &entry : routeID_str_tokenize)
        {
            boost::trim(entry);
            if(entry == "")
                throw omnetpp::cRuntimeError("'route' attribute is not formatted correctly in %s node", vehicle_flow_tag.c_str());
        }

        std::string routeDist_str = "";
        std::vector<double> routeDist_tokenize;

        // we have multiple routes
        if(routeID_str_tokenize.size() > 1)
        {
            cAttr = cNode->first_attribute("routeDist");
            if(!cAttr)
                throw omnetpp::cRuntimeError("'routeDist' attribute is not found in %s node", vehicle_flow_tag.c_str());
            routeDist_str = cAttr->value();
            boost::trim(routeDist_str);

            // tokenize routeDist_str
            std::vector<std::string> routeDist_str_tokenize;
            boost::split(routeDist_str_tokenize, routeDist_str, boost::is_any_of(","));

            if(routeID_str_tokenize.size() != routeDist_str_tokenize.size())
                throw omnetpp::cRuntimeError("'route' and 'routeDist' attributes do not match in %s node", vehicle_flow_tag.c_str());

            double sum = 0;
            for(auto &entry : routeDist_str_tokenize)
            {
                boost::trim(entry);

                double routeDist_e = 0;
                try
                {
                    routeDist_e = boost::lexical_cast<double>(entry);
                }
                catch (boost::bad_lexical_cast const&)
                {
                    throw omnetpp::cRuntimeError("'routeDist' attribute is badly formatted in %s node: %s", vehicle_flow_tag.c_str(), routeDist_str.c_str());
                }

                routeDist_tokenize.push_back(routeDist_e);
                sum += routeDist_e;
            }

            if(sum != 100)
                throw omnetpp::cRuntimeError("'routeDist' values do not add up to 100 percent in %s node: %s", vehicle_flow_tag.c_str(), routeDist_str.c_str());
        }

        std::string speed_str = "0";
        cAttr = cNode->first_attribute("speed");
        if(cAttr)
        {
            speed_str = cAttr->value();
            boost::trim(speed_str);
        }

        double speed = 0;
        try
        {
            speed = boost::lexical_cast<double>(speed_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'speed' attribute is badly formatted in %s node: %s", vehicle_flow_tag.c_str(), speed_str.c_str());
        }

        std::string lane_str = "-5";  // DEPART_LANE_BEST_FREE
        cAttr = cNode->first_attribute("lane");
        if(cAttr)
        {
            lane_str = cAttr->value();
            boost::trim(lane_str);
        }

        int lane = 0;
        try
        {
            lane = boost::lexical_cast<int>(lane_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'lane' attribute is badly formatted in %s node: %s", vehicle_flow_tag.c_str(), lane_str.c_str());
        }

        std::string lanePos_str = "0";
        cAttr = cNode->first_attribute("lanePos");
        if(cAttr)
        {
            lanePos_str = cAttr->value();
            boost::trim(lanePos_str);
        }

        double lanePos = 0;
        try
        {
            lanePos = boost::lexical_cast<double>(lanePos_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'lanePos' attribute is badly formatted in %s node: %s", vehicle_flow_tag.c_str(), lanePos_str.c_str());
        }

        std::string laneChangeMode_str = std::to_string(LANECHANGEMODE_DEFAULT);
        cAttr = cNode->first_attribute("laneChangeMode");
        if(cAttr)
        {
            laneChangeMode_str = cAttr->value();
            boost::trim(laneChangeMode_str);
        }

        int laneChangeMode = 0;
        try
        {
            laneChangeMode = boost::lexical_cast<int>(laneChangeMode_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'laneChangeMode' attribute is badly formatted in %s node: %s", vehicle_flow_tag.c_str(), laneChangeMode_str.c_str());
        }

        if(laneChangeMode < 0)
            throw omnetpp::cRuntimeError("'laneChangeMode' value is not valid in %s node: %s", vehicle_flow_tag.c_str(), laneChangeMode_str.c_str());

        std::string begin_str = "0";
        cAttr = cNode->first_attribute("begin");
        if(cAttr)
        {
            begin_str = cAttr->value();
            boost::trim(begin_str);
        }

        double begin = 0;
        try
        {
            begin = boost::lexical_cast<double>(begin_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'begin' attribute is badly formatted in %s node: %s", vehicle_flow_tag.c_str(), begin_str.c_str());
        }

        if(begin < 0)
            throw omnetpp::cRuntimeError("'begin' value is negative in %s node: %s", vehicle_flow_tag.c_str(), begin_str.c_str());

        int number = -1;
        double end = -1;

        // the form is: begin, number
        if((cAttr = cNode->first_attribute("number")))
        {
            std::string number_str = cAttr->value();
            boost::trim(number_str);

            try
            {
                number = boost::lexical_cast<int>(number_str);
            }
            catch (boost::bad_lexical_cast const&)
            {
                throw omnetpp::cRuntimeError("'number' attribute is badly formatted in %s node: %s", vehicle_flow_tag.c_str(), number_str.c_str());
            }

            if(number < 0)
                throw omnetpp::cRuntimeError("'number' value is negative in %s node: %s", vehicle_flow_tag.c_str(), number_str.c_str());

            cAttr = cNode->first_attribute("end");
            if(cAttr)
                throw omnetpp::cRuntimeError("'end' attribute is redundant when 'number' is present in %s node", vehicle_flow_tag.c_str());
        }
        // the form is: begin, end
        else if((cAttr = cNode->first_attribute("end")))
        {
            std::string end_str = cAttr->value();
            boost::trim(end_str);

            try
            {
                end = boost::lexical_cast<double>(end_str);
            }
            catch (boost::bad_lexical_cast const&)
            {
                throw omnetpp::cRuntimeError("'end' attribute is badly formatted in %s node: %s", vehicle_flow_tag.c_str(), end_str.c_str());
            }

            if(end <= begin)
                throw omnetpp::cRuntimeError("'end' value is smaller than 'begin' value in %s node", vehicle_flow_tag.c_str());

            cAttr = cNode->first_attribute("number");
            if(cAttr)
                throw omnetpp::cRuntimeError("'number' attribute is redundant when 'end' is present in %s node", vehicle_flow_tag.c_str());
        }
        else
            throw omnetpp::cRuntimeError("either 'number' or 'end' attributes should be defined in %s node", vehicle_flow_tag.c_str());

        std::string seed_str = "0";
        cAttr = cNode->first_attribute("seed");
        if(cAttr)
        {
            seed_str = cAttr->value();
            boost::trim(seed_str);
        }

        int seed = 0;
        try
        {
            seed = boost::lexical_cast<int>(seed_str);
        }
        catch (boost::bad_lexical_cast const&)
        {
            throw omnetpp::cRuntimeError("'seed' attribute is badly formatted in %s node: %s", vehicle_flow_tag.c_str(), seed_str.c_str());
        }

        if(seed < 0)
            throw omnetpp::cRuntimeError("'seed' value should be positive in %s node: %s", vehicle_flow_tag.c_str(), seed_str.c_str());

        cAttr = cNode->first_attribute("distribution");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'distribution' attribute is not found in %s node", vehicle_flow_tag.c_str());
        std::string distribution_str = cAttr->value();
        boost::trim(distribution_str);

        double period = 0;
        double lambda = 0;
        double probability = 0;

        // deterministic distribution needs 'period'
        if(distribution_str == "deterministic")
        {
            cAttr = cNode->first_attribute("lambda");
            if(cAttr)
                throw omnetpp::cRuntimeError("'lambda' attribute is redundant in deterministic distribution in %s node", vehicle_flow_tag.c_str());

            cAttr = cNode->first_attribute("probability");
            if(cAttr)
                throw omnetpp::cRuntimeError("'probability' attribute is redundant in deterministic distribution in %s node", vehicle_flow_tag.c_str());

            cAttr = cNode->first_attribute("period");
            if(!cAttr)
                throw omnetpp::cRuntimeError("'period' attribute is not found in %s node", vehicle_flow_tag.c_str());
            std::string period_str = cAttr->value();
            boost::trim(period_str);

            try
            {
                period = boost::lexical_cast<double>(period_str);
            }
            catch (boost::bad_lexical_cast const&)
            {
                throw omnetpp::cRuntimeError("'period' attribute is badly formatted in %s node: %s", vehicle_flow_tag.c_str(), period_str.c_str());
            }

            if(period < 0)
                throw omnetpp::cRuntimeError("'period' value is negative in %s node: %s", vehicle_flow_tag.c_str(), period_str.c_str());
        }
        // poisson distribution needs 'lambda' and 'seed'
        else if(distribution_str == "poisson")
        {
            cAttr = cNode->first_attribute("period");
            if(cAttr)
                throw omnetpp::cRuntimeError("'period' attribute is redundant in poisson distribution in %s node", vehicle_flow_tag.c_str());

            cAttr = cNode->first_attribute("probability");
            if(cAttr)
                throw omnetpp::cRuntimeError("'probability' attribute is redundant in poisson distribution in %s node", vehicle_flow_tag.c_str());

            cAttr = cNode->first_attribute("lambda");
            if(!cAttr)
                throw omnetpp::cRuntimeError("'lambda' attribute is not found in %s node", vehicle_flow_tag.c_str());
            std::string lambda_str = cAttr->value();
            boost::trim(lambda_str);

            try
            {
                lambda = boost::lexical_cast<double>(lambda_str);
            }
            catch (boost::bad_lexical_cast const&)
            {
                throw omnetpp::cRuntimeError("'lambda' attribute is badly formatted in %s node: %s", vehicle_flow_tag.c_str(), lambda_str.c_str());
            }

            if(lambda <= 0)
                throw omnetpp::cRuntimeError("'lambda' value should be positive in %s node: %s", vehicle_flow_tag.c_str(), lambda_str.c_str());
        }
        // uniform distribution needs 'probability'
        else if(distribution_str == "uniform")
        {
            cAttr = cNode->first_attribute("lambda");
            if(cAttr)
                throw omnetpp::cRuntimeError("'lambda' attribute is redundant in uniform distribution in %s node", vehicle_flow_tag.c_str());

            cAttr = cNode->first_attribute("period");
            if(cAttr)
                throw omnetpp::cRuntimeError("'period' attribute is redundant in uniform distribution in %s node", vehicle_flow_tag.c_str());

            cAttr = cNode->first_attribute("probability");
            if(!cAttr)
                throw omnetpp::cRuntimeError("'probability' attribute is not found in %s node", vehicle_flow_tag.c_str());
            std::string probability_str = cAttr->value();
            boost::trim(probability_str);

            try
            {
                probability = boost::lexical_cast<double>(probability_str);
            }
            catch (boost::bad_lexical_cast const&)
            {
                throw omnetpp::cRuntimeError("'probability' attribute is badly formatted in %s node: %s", vehicle_flow_tag.c_str(), probability_str.c_str());
            }

            if(probability < 0 || probability > 1)
                throw omnetpp::cRuntimeError("'probability' should be in range [0,1] in %s node: %s", vehicle_flow_tag.c_str(), probability_str.c_str());
        }
        else
            throw omnetpp::cRuntimeError("'distribution' value is invalid in %s node: %s", vehicle_flow_tag.c_str(), distribution_str.c_str());

        auto it = allVehicleFlow.find(id_str);
        if(it == allVehicleFlow.end())
        {
            vehicleFlowEntry_t entry = {};

            entry.id_str = id_str;
            entry.type_str_tokenize = type_str_tokenize;
            entry.typeDist_tokenize = typeDist_tokenize;
            entry.color_str = color_str;
            entry.routeID_str_tokenize = routeID_str_tokenize;
            entry.routeDist_tokenize = routeDist_tokenize;
            entry.speed = speed;
            entry.lane = lane;
            entry.lanePos = lanePos;
            entry.laneChangeMode = laneChangeMode;
            entry.number = number;
            entry.begin = begin;
            entry.end = end;
            entry.seed = seed;
            entry.distribution_str = distribution_str;
            entry.period = period;
            entry.lambda = lambda;
            entry.probability = probability;

            allVehicleFlow.insert(std::make_pair(id_str, entry));
        }
        else
            throw omnetpp::cRuntimeError("Multiple %s with the same 'id' %s is not allowed!", vehicle_flow_tag.c_str(), id_str.c_str());
    }
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
        // each flow has its own seed/generator
        // mersenne twister engine -- choose a fix seed to make tests reproducible
        std::mt19937 generator(entry.second.seed);
        // generating a random floating point number uniformly in [1,0)
        std::uniform_real_distribution<> vehTypeDist(0,1);
        // generating a random floating point number uniformly in [1,0)
        std::uniform_real_distribution<> vehRouteDist(0,1);

        if(entry.second.number == -1 && entry.second.end == -1)
            throw omnetpp::cRuntimeError("'number' and 'end' attributes are both unknown in %s node", vehicle_flow_tag.c_str());

        // note than begin and end are in seconds
        double begin = entry.second.begin;
        double end = (entry.second.end != -1) ? entry.second.end : std::numeric_limits<double>::max();
        end = std::min(end, terminate); // end can not exceeds the terminate

        int maxVehNum = (entry.second.number != -1) ? entry.second.number : std::numeric_limits<int>::max();

        if(entry.second.distribution_str == "deterministic")
        {
            double depart = begin;

            // for each vehicle
            for(int veh = 0; veh < maxVehNum; veh++)
            {
                if(depart >= end)
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
                TraCI->vehicleAdd(vehID, vehType, vehRoute, (int32_t)(depart*1000), entry.second.lanePos, entry.second.speed, entry.second.lane);

                // change its color
                RGB newColor = Color::colorNameToRGB(entry.second.color_str);
                TraCI->vehicleSetColor(vehID, newColor);

                // change lane change mode
                if(entry.second.laneChangeMode != LANECHANGEMODE_DEFAULT)
                    TraCI->vehicleSetLaneChangeMode(vehID, entry.second.laneChangeMode);

                depart += entry.second.period;
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
            for(double depart = begin ; depart < end; depart += SUMO_timeStep)
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
                    TraCI->vehicleAdd(vehID, vehType, vehRoute, (int32_t)(depart*1000), entry.second.lanePos, entry.second.speed, entry.second.lane);

                    // change its color
                    RGB newColor = Color::colorNameToRGB(entry.second.color_str);
                    TraCI->vehicleSetColor(vehID, newColor);

                    // change lane change mode
                    if(entry.second.laneChangeMode != LANECHANGEMODE_DEFAULT)
                        TraCI->vehicleSetLaneChangeMode(vehID, entry.second.laneChangeMode);

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


std::string AddNode::getVehRoute(vehicleFlowEntry_t entry, double rnd)
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


void AddNode::parseEmulated(rapidxml::xml_node<> *pNode)
{
    // Iterate over all 'emulated' nodes
    for(rapidxml::xml_node<> *cNode = pNode->first_node(emulated_tag.c_str()); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != emulated_tag)
            continue;

        // format checking: Iterate over all attributes in this node
        for(rapidxml::xml_attribute<> *cAttr1 = cNode->first_attribute(); cAttr1; cAttr1 = cAttr1->next_attribute())
        {
            std::string attName = cAttr1->name();

            if(attName != "id" &&
                    attName != "ip" &&
                    attName != "color")
                throw omnetpp::cRuntimeError("'%s' is not a valid attribute in node '%s'", attName.c_str(), emulated_tag.c_str());
        }

        rapidxml::xml_attribute<> *cAttr = cNode->first_attribute("id");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'id' attribute is not found in %s node", emulated_tag.c_str());
        std::string id_str = cAttr->value();
        boost::trim(id_str);

        cAttr = cNode->first_attribute("ip");
        if(!cAttr)
            throw omnetpp::cRuntimeError("'ip' attribute is not found in %s node", emulated_tag.c_str());
        std::string ip_str = cAttr->value();
        boost::trim(ip_str);

        std::string color_str = "yellow";
        cAttr = cNode->first_attribute("color");
        if(cAttr)
        {
            color_str = cAttr->value();
            boost::trim(color_str);
        }

        auto it = allEmulated.find(id_str);
        if(it == allEmulated.end())
        {
            emulatedEntry entry = {};

            entry.id_str = id_str;
            entry.ip_str = ip_str;
            entry.color_str = color_str;

            allEmulated.insert(std::make_pair(id_str, entry));
        }
        else
            throw omnetpp::cRuntimeError("Multiple %s with the same 'id' %s is not allowed!", emulated_tag.c_str(), id_str.c_str());
    }
}


void AddNode::addEmulated()
{
    if(allEmulated.empty())
        return;

    unsigned int num = allEmulated.size();

    LOG_DEBUG << boost::format("\n>>> AddNode is marking %1% nodes as emulated ... \n") % num << std::flush;

    auto loadedVehList = TraCI->simulationGetLoadedVehiclesIDList();

    for(auto &entry : allEmulated)
    {
        std::string vehID = entry.second.id_str;

        // make sure the emulated vehicle is one of the inserted vehicles
        auto it = std::find(loadedVehList.begin(), loadedVehList.end(), vehID);
        if(it == loadedVehList.end())
            throw omnetpp::cRuntimeError("Node '%s' marked as emulated does not exist", vehID.c_str());

        TraCI->add2Emulated(vehID, entry.second.ip_str);

        // change its color
        RGB newColor = Color::colorNameToRGB(entry.second.color_str);
        TraCI->vehicleSetColor(vehID, newColor);
    }
}


void AddNode::addCircle(std::string name, std::string type, const RGB color, bool filled, Coord *center, double radius)
{
    std::list<TraCICoord> circlePoints;

    // Convert from degrees to radians via multiplication by PI/180
    for(int angleInDegrees = 0; angleInDegrees <= 360; angleInDegrees += 10)
    {
        double x = (double)( radius * cos(angleInDegrees * 3.14 / 180) ) + center->x;
        double y = (double)( radius * sin(angleInDegrees * 3.14 / 180) ) + center->y;

        circlePoints.push_back(TraCICoord(x, y));
    }

    // create polygon in SUMO
    TraCI->polygonAddTraCI(name, type, color, filled /*filled*/, 1 /*layer*/, circlePoints);
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
    LOG_DEBUG << boost::format("  %1% vehicle/bike types are loaded: \n      ") % loadedVehTypeList.size();
    for(std::string type : loadedVehTypeList)
        LOG_DEBUG << boost::format("%1%, ") % type;

    LOG_DEBUG << "\n";

    //#############################
    // Get the list of all vehicles
    //#############################

    auto loadedVehList = TraCI->simulationGetLoadedVehiclesIDList();
    LOG_DEBUG << boost::format("  %1% vehicles/bikes are loaded: \n") % loadedVehList.size();
    // get vehicle/bike type distribution
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

}

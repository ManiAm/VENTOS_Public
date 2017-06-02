/****************************************************************************/
/// @file    TrafficControl.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Jun 2017
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

#include "TrafficControl.h"
#include "logging/VENTOS_logging.h"
#include "xmlUtil.h"

namespace VENTOS {

Define_Module(VENTOS::TrafficControl);

TrafficControl::~TrafficControl()
{

}


void TrafficControl::initialize(int stage)
{
    super::initialize(stage);

    if(stage == 0)
    {
        id = par("id").stringValue();
        if(id == "")
            return;

        // get a pointer to the TraCI module
        TraCI = TraCI_Commands::getTraCI();

        Signal_initialize_withTraCI = registerSignal("initializeWithTraCISignal");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initializeWithTraCISignal", this);
    }
}


void TrafficControl::finish()
{
    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("initializeWithTraCISignal", this);
}


void TrafficControl::handleMessage(omnetpp::cMessage *msg)
{
    throw omnetpp::cRuntimeError("Cannot handle msg '%s' of type '%d'", msg->getFullName(), msg->getKind());
}


void TrafficControl::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        readInsertion("trafficControl.xml");
    }
}


void TrafficControl::readInsertion(std::string addNodePath)
{
    rapidxml::file<> xmlFile(addNodePath.c_str());  // Convert our file to a rapid-xml readable object
    rapidxml::xml_document<> doc;                   // Build a rapidxml doc
    doc.parse<0>(xmlFile.data());                   // Fill it with data from our file

    // Get the first applDependency node
    rapidxml::xml_node<> *pNode = doc.first_node("trafficControl");

    if(pNode == NULL)
    {
        LOG_WARNING << boost::format("\nWARNING: There is no 'trafficControl' nodes in the trafficControl.xml file \n") << std::flush;
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
                throw omnetpp::cRuntimeError("Cannot find id '%s' in the trafficControl.xml file!", this->id.c_str());
        }
    }

    // format checking: Iterate over all nodes in this id
    for(rapidxml::xml_node<> *cNode = pNode->first_node(); cNode; cNode = cNode->next_sibling())
    {
        std::string nodeName = cNode->name();

        if(nodeName != adversary_tag)
            throw omnetpp::cRuntimeError("'%s' is not a valid node in id '%s'", this->id.c_str());
    }

    parseAdversary(pNode);

    if(allAdversary.empty())
        LOG_WARNING << boost::format("\nWARNING: Add node with id '%1%' is empty! \n") % this->id << std::flush;

    addAdversary();
}



void TrafficControl::parseAdversary(rapidxml::xml_node<> *pNode)
{
    // Iterate over all 'adversary' nodes
    for(rapidxml::xml_node<> *cNode = pNode->first_node(adversary_tag.c_str()); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != adversary_tag)
            continue;

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
}


void TrafficControl::addAdversary()
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
    for(auto &entry : allAdversary)
    {

    }
}

}

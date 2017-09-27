/****************************************************************************/
/// @file    gui.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    June 2017
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

#include <algorithm>
#include <regex>
#include <boost/algorithm/string.hpp>

#include "gui.h"
#include "logging/VENTOS_logging.h"
#include "xmlUtil.h"

Define_Module(VENTOS::gui);

namespace VENTOS {

gui::~gui()
{

}


void gui::initialize(int stage)
{
    if(stage == 0)
    {
        id = par("id").stringValue();
        if(id == "")
            return;

        // get a pointer to the TraCI module
        TraCI = TraCI_Commands::getTraCI();

        if(!TraCI->IsGUI())
            return;

        Signal_initialize_withTraCI = registerSignal("initializeWithTraCISignal");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initializeWithTraCISignal", this);

        Signal_executeEachTS = registerSignal("executeEachTimeStepSignal");
        omnetpp::getSimulation()->getSystemModule()->subscribe("executeEachTimeStepSignal", this);
    }
}


void gui::finish()
{
    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("initializeWithTraCISignal", this);
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("executeEachTimeStepSignal", this);
}


void gui::handleMessage(omnetpp::cMessage *msg)
{
    throw omnetpp::cRuntimeError("Cannot handle msg '%s' of type '%d'", msg->getFullName(), msg->getKind());
}


void gui::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        SUMO_timeStep = (double)TraCI->simulationGetDelta() / 1000.;
        if(SUMO_timeStep <= 0)
            throw omnetpp::cRuntimeError("Simulation time step '%d' is invalid", SUMO_timeStep);

        readInsertion("gui.xml");
    }
    else if(signalID == Signal_executeEachTS)
    {
        if(!allViewport.empty())
            controlViewport();

        if(!allTracking.empty())
            controlTracking();
    }
}


void gui::readInsertion(std::string addNodePath)
{
    rapidxml::file<> xmlFile(addNodePath.c_str());  // Convert our file to a rapid-xml readable object
    rapidxml::xml_document<> doc;                   // Build a rapidxml doc
    doc.parse<0>(xmlFile.data());                   // Fill it with data from our file

    // Get the first gui node
    rapidxml::xml_node<> *pNode = doc.first_node("gui");

    if(pNode == NULL)
    {
        LOG_WARNING << boost::format("\nWARNING: There is no 'gui' nodes in the gui.xml file \n") << std::flush;
        return;
    }

    while(1)
    {
        // Get id attribute
        rapidxml::xml_attribute<> *pAttr = pNode->first_attribute("id");

        // Get the value of this attribute
        std::string strValue = pAttr->value();

        // We found the correct id
        if(strValue == this->id)
            break;
        // Get the next id
        else
        {
            pNode = pNode->next_sibling();
            if(!pNode)
                throw omnetpp::cRuntimeError("Cannot find id '%s' in the gui.xml file!", this->id.c_str());
        }
    }

    // format checking: Iterate over all nodes in this id
    for(rapidxml::xml_node<> *cNode = pNode->first_node(); cNode; cNode = cNode->next_sibling())
    {
        std::string nodeName = cNode->name();

        if(nodeName != viewport_tag && nodeName != track_tag)
            throw omnetpp::cRuntimeError("'%s' is not a valid element in id '%s' of gui.xml file!", nodeName.c_str(), this->id.c_str());
    }

    parseViewport(pNode);
    parseTracking(pNode);

    if(allViewport.empty() && allTracking.empty())
        LOG_WARNING << boost::format("\nWARNING: GUI with id '%1%' is empty! \n") % this->id << std::flush;

    controlViewport();
    controlTracking();
}


void gui::parseViewport(rapidxml::xml_node<> *pNode)
{
    uint32_t nodeCount = 1;

    // Iterate over all 'viewport' nodes
    for(rapidxml::xml_node<> *cNode = pNode->first_node(viewport_tag.c_str()); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != viewport_tag)
            continue;

        std::vector<std::string> validAttr = {"viewId", "zoom", "offsetX", "offsetY", "begin", "steps"};
        xmlUtil::validityCheck(cNode, validAttr);

        std::string viewId_str = xmlUtil::getAttrValue_string(cNode, "viewId", false, "View #0");
        double zoom = xmlUtil::getAttrValue_double(cNode, "zoom", false, 100);
        double offsetX = xmlUtil::getAttrValue_double(cNode, "offsetX", false, std::numeric_limits<int>::max());
        double offsetY = xmlUtil::getAttrValue_double(cNode, "offsetY", false, std::numeric_limits<int>::max());
        double begin = xmlUtil::getAttrValue_double(cNode, "begin", false, 0);
        int steps = xmlUtil::getAttrValue_int(cNode, "steps", false, 0);

        // make sure the view ID is in the 'View #n' format
        if (!std::regex_match (viewId_str, std::regex("(View #)([[:digit:]]+)") ))
            throw omnetpp::cRuntimeError("attribute 'viewId' is not in the 'View #n' format in element '%s'", viewport_tag.c_str());

        if(zoom < 100)
            throw omnetpp::cRuntimeError("attribute 'zoom' should be >=100 in element '%s'", viewport_tag.c_str());

        if(begin < 0)
            throw omnetpp::cRuntimeError("attribute 'begin' should be positive in element '%s'", viewport_tag.c_str());

        if(steps < 0)
            throw omnetpp::cRuntimeError("attribute 'steps' should be positive in element '%s'", viewport_tag.c_str());

        // make sure at least one attribute is present
        if(!cNode->first_attribute("zoom") &&
                !cNode->first_attribute("offsetX") &&
                !cNode->first_attribute("offsetY"))
            throw omnetpp::cRuntimeError("at least one of the 'zoom/offsetX/offsetY' attributes should be present in element '%s'", viewport_tag.c_str());

        auto it = allViewport.find(nodeCount);
        if(it == allViewport.end())
        {
            viewportEntry entry = {};

            entry.viewId_str = viewId_str;
            entry.zoom = zoom;
            entry.offsetX = offsetX;
            entry.offsetY = offsetY;
            entry.begin = begin;
            entry.steps = steps;

            allViewport.insert(std::make_pair(nodeCount, entry));
        }
        else
            throw omnetpp::cRuntimeError("Multiple %s with the same 'id' is not allowed!", viewport_tag.c_str());

        nodeCount++;
    }
}


void gui::controlViewport()
{
    for(auto &entry : allViewport)
    {
        if(entry.second.processingEnded)
            continue;

        ASSERT(entry.second.begin >= 0);
        ASSERT(entry.second.viewId_str != "");

        // wait until 'begin'
        if(entry.second.begin > omnetpp::simTime().dbl())
            continue;

        if(!entry.second.processingStarted)
        {
            // create the view if not exist
            TraCI->GUIAddView(entry.second.viewId_str);

            // adjust both X and Y offsets
            if(entry.second.offsetX != std::numeric_limits<int>::max() && entry.second.offsetY != std::numeric_limits<int>::max())
            {
                TraCI->GUISetOffset(entry.second.viewId_str, entry.second.offsetX, entry.second.offsetY);
            }
            // adjust X offset
            else if(entry.second.offsetX != std::numeric_limits<int>::max())
            {
                TraCICoord currentOffset = TraCI->GUIGetOffset(entry.second.viewId_str);
                TraCI->GUISetOffset(entry.second.viewId_str, entry.second.offsetX, currentOffset.y);
            }
            // adjust Y offset
            else if(entry.second.offsetY != std::numeric_limits<int>::max())
            {
                TraCICoord currentOffset = TraCI->GUIGetOffset(entry.second.viewId_str);
                TraCI->GUISetOffset(entry.second.viewId_str, currentOffset.x, entry.second.offsetY);
            }

            if(entry.second.steps == 0)
            {
                // zoom in
                TraCI->GUISetZoom(entry.second.viewId_str, entry.second.zoom);

                // we are done!
                entry.second.processingStarted = true;
                entry.second.processingEnded = true;
                continue;
            }
            else
            {
                // get the current zoom level
                entry.second.baseZoom = TraCI->GUIGetZoom(entry.second.viewId_str);

                // calculate how many zoom in/out steps do we need
                entry.second.zoomSteps = (entry.second.zoom - entry.second.baseZoom) / entry.second.steps;

                entry.second.processingStarted = true;
            }
        }

        // start from the base zoom level
        static double currentZoom = entry.second.baseZoom;
        // and gradually increase the zoom
        currentZoom += entry.second.zoomSteps;

        TraCI->GUISetZoom(entry.second.viewId_str, currentZoom);

        if(entry.second.zoomSteps == 0)
        {
            // we are done
            entry.second.processingEnded = true;
        }
        // we are zooming in
        else if(entry.second.zoomSteps > 0)
        {
            // stop the zooming?
            if(currentZoom >= entry.second.zoom)
                entry.second.processingEnded = true;
        }
        // we are zooming out
        else if(entry.second.zoomSteps < 0)
        {
            // stop the zooming?
            if(currentZoom <= entry.second.zoom)
                entry.second.processingEnded = true;
        }
    }
}


void gui::parseTracking(rapidxml::xml_node<> *pNode)
{
    uint32_t nodeCount = 1;

    // Iterate over all 'track' nodes
    for(rapidxml::xml_node<> *cNode = pNode->first_node(track_tag.c_str()); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != track_tag)
            continue;

        std::vector<std::string> validAttr = {"vehId", "viewId", "begin", "updateRate"};
        xmlUtil::validityCheck(cNode, validAttr);

        std::string viewId_str = xmlUtil::getAttrValue_string(cNode, "viewId", false, "View #0");
        std::string vehId_str = xmlUtil::getAttrValue_string(cNode, "vehId", false, "");
        double begin = xmlUtil::getAttrValue_double(cNode, "begin", false, 0);
        double updateRate = xmlUtil::getAttrValue_double(cNode, "updateRate", false, -1);

        // make sure the view ID is in the 'View #n' format
        if (!std::regex_match (viewId_str, std::regex("(View #)([[:digit:]]+)") ))
            throw omnetpp::cRuntimeError("attribute 'viewId' is not in the 'View #n' format in element '%s'", viewport_tag.c_str());

        if(begin < 0)
            throw omnetpp::cRuntimeError("attribute 'begin' should be positive in element '%s'", track_tag.c_str());

        if(updateRate != -1 && updateRate <= 0)
            throw omnetpp::cRuntimeError("attribute 'updateRate' should be positive in element '%s'", track_tag.c_str());

        // make sure at least one attribute is present
        if(!cNode->first_attribute("vehId") && !cNode->first_attribute("pltId"))
            throw omnetpp::cRuntimeError("at least one of the 'vehId/pltId' attributes should be present in element '%s'", track_tag.c_str());

        auto it = allTracking.find(nodeCount);
        if(it == allTracking.end())
        {
            trackEntry_t entry = {};

            entry.viewId_str = viewId_str;
            entry.vehId_str = vehId_str;
            entry.begin = begin;
            entry.updateRate = updateRate;

            allTracking.insert(std::make_pair(nodeCount, entry));
        }
        else
            throw omnetpp::cRuntimeError("Multiple %s with the same 'id' is not allowed!", track_tag.c_str());

        nodeCount++;
    }
}


void gui::controlTracking()
{
    for(auto &entry : allTracking)
    {
        if(entry.second.processingEnded)
            continue;

        ASSERT(entry.second.begin >= 0);
        ASSERT(entry.second.viewId_str != "");

        // wait until 'begin'
        if(entry.second.begin > omnetpp::simTime().dbl())
            continue;

        if(!entry.second.processingStarted)
        {
            auto ii = TraCI->departureArrival.find(entry.second.vehId_str);
            if(ii == TraCI->departureArrival.end())
            {
                // the vehicle hasn't departed yet
                continue;
            }
            else if(ii->second.arrival != -1)
            {
                LOG_WARNING << boost::format("\nWARNING: Vehicle '%s' tracking is not possible, because it has left the network at time '%f' \n ") %
                        entry.second.vehId_str %
                        ii->second.arrival << std::flush;
                entry.second.processingEnded = true;
                continue;
            }

            // create the view if not exist
            TraCI->GUIAddView(entry.second.viewId_str);

            TraCI->GUISetTrackVehicle(entry.second.viewId_str, entry.second.vehId_str);

            entry.second.processingStarted = true;
        }

        entry.second.processingEnded = true;
    }
}

}

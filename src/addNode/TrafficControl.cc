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

        Signal_executeEachTS = registerSignal("executeEachTimeStepSignal");
        omnetpp::getSimulation()->getSystemModule()->subscribe("executeEachTimeStepSignal", this);
    }
}


void TrafficControl::finish()
{
    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("initializeWithTraCISignal", this);
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("executeEachTimeStepSignal", this);
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
        SUMO_timeStep = (double)TraCI->simulationGetTimeStep() / 1000.;

        readInsertion("trafficControl.xml");
    }
    else if(signalID == Signal_executeEachTS)
    {
        if(!allSpeed.empty())
            controlSpeed();

        if(!allOptSize.empty())
            controlOptSize();

        if(!allPltMerge.empty())
            controlPltMerge();

        if(!allPltSplit.empty())
            controlPltSplit();
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

        if(nodeName != speed_tag &&
                nodeName != optSize_tag &&
                nodeName != pltMerge_tag &&
                nodeName != pltSplit_tag)
            throw omnetpp::cRuntimeError("'%s' is not a valid node in id '%s'", nodeName.c_str(), this->id.c_str());
    }

    parseSpeed(pNode);
    parseOptSize(pNode);
    parsePltMerge(pNode);
    parsePltSplit(pNode);

    if(allSpeed.empty() && allOptSize.empty() && allPltMerge.empty() && allPltSplit.empty())
        LOG_WARNING << boost::format("\nWARNING: Traffic control with id '%1%' is empty! \n") % this->id << std::flush;

    controlSpeed();
    controlOptSize();
    controlPltMerge();
    controlPltSplit();
}



void TrafficControl::parseSpeed(rapidxml::xml_node<> *pNode)
{
    uint32_t nodeCount = 1;

    auto allEdges = TraCI->edgeGetIDList();
    auto allLanes = TraCI->laneGetIDList();

    // Iterate over all 'speed' nodes
    for(rapidxml::xml_node<> *cNode = pNode->first_node(speed_tag.c_str()); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != speed_tag)
            continue;

        std::vector<std::string> validAttr = {"id", "begin", "end", "duration", "edgeId", "edgePos", "laneId",
                "lanePos", "value", "maxAccel", "maxDecel"};
        xmlUtil::validityCheck(cNode, validAttr);

        std::string id_str = xmlUtil::getAttrValue_string(cNode, "id");
        double begin = xmlUtil::getAttrValue_double(cNode, "begin", false, -1);
        double end = xmlUtil::getAttrValue_double(cNode, "end", false, -1);
        double duration = xmlUtil::getAttrValue_double(cNode, "duration", false, -1);
        std::string edgeId_str = xmlUtil::getAttrValue_string(cNode, "edgeId", false, "");
        double edgePos = xmlUtil::getAttrValue_double(cNode, "edgePos", false, 0);
        std::string laneId_str = xmlUtil::getAttrValue_string(cNode, "laneId", false, "");
        double lanePos = xmlUtil::getAttrValue_double(cNode, "lanePos", false, 0);
        std::string value_str = xmlUtil::getAttrValue_string(cNode, "value");
        double maxAccel = xmlUtil::getAttrValue_double(cNode, "maxAccel", false, -1);
        double maxDecel = xmlUtil::getAttrValue_double(cNode, "maxDecel", false, -1);

        if(begin != -1 && begin < 0)
            throw omnetpp::cRuntimeError("attribute 'begin' cannot be negative in element '%s'", speed_tag.c_str());

        if(end != -1 && begin != -1 && end <= begin)
            throw omnetpp::cRuntimeError("attribute 'end' cannot be lower than 'begin' in element '%s'", speed_tag.c_str());

        if(duration != -1 && duration <= 0)
            throw omnetpp::cRuntimeError("attribute 'duration' cannot be negative in element '%s'", speed_tag.c_str());

        if(cNode->first_attribute("end") && cNode->first_attribute("duration"))
            throw omnetpp::cRuntimeError("attribute 'duration' and 'end' cannot be present together in element '%s'", speed_tag.c_str());

        if(maxAccel != -1 && maxAccel < 0)
            throw omnetpp::cRuntimeError("attribute 'maxAccel' cannot be negative in element '%s'", speed_tag.c_str());

        if(maxDecel != -1 && maxDecel < 0)
            throw omnetpp::cRuntimeError("attribute 'maxDecel' cannot be negative in element '%s'", speed_tag.c_str());

        if(cNode->first_attribute("edgeId") && (cNode->first_attribute("laneId") || cNode->first_attribute("begin")))
            throw omnetpp::cRuntimeError("attribute 'laneId/begin' is redundant when 'edgeId' is present in element '%s'", speed_tag.c_str());

        if(cNode->first_attribute("laneId") && (cNode->first_attribute("edgeId") || cNode->first_attribute("begin")))
            throw omnetpp::cRuntimeError("attribute 'edgeId/begin' is redundant when 'laneId' is present in element '%s'", speed_tag.c_str());

        if(cNode->first_attribute("begin") && (cNode->first_attribute("edgeId") || cNode->first_attribute("laneId")))
            throw omnetpp::cRuntimeError("attribute 'edgeId/laneId' is redundant when 'begin' is present in element '%s'", speed_tag.c_str());

        if(!cNode->first_attribute("begin") && !cNode->first_attribute("edgeId") && !cNode->first_attribute("laneId"))
            throw omnetpp::cRuntimeError("One of 'begin', 'edgeId' or 'laneId' attributes should be defined in element '%s'", speed_tag.c_str());

        if(cNode->first_attribute("edgePos") && !cNode->first_attribute("edgeId"))
            throw omnetpp::cRuntimeError("attribute 'edgePos' requires 'edgeId' in element '%s'", speed_tag.c_str());

        if(cNode->first_attribute("lanePos") && !cNode->first_attribute("laneId"))
            throw omnetpp::cRuntimeError("attribute 'lanePos' requires 'laneId' in element '%s'", speed_tag.c_str());

        try
        {
            boost::lexical_cast<double>(value_str);
        }
        catch(...)
        {
            // speed value is a function
            if(!cNode->first_attribute("end") && !cNode->first_attribute("duration"))
                throw omnetpp::cRuntimeError("attribute 'end' or 'duration' is mandatory when speed is a function in element '%s'", speed_tag.c_str());
        }

        if(edgeId_str != "")
        {
            auto ii = std::find(allEdges.begin(), allEdges.end(), edgeId_str);
            if(ii == allEdges.end())
                throw omnetpp::cRuntimeError("Edge '%s' is not a valid edge in element '%s'", edgeId_str.c_str(), speed_tag.c_str());

            // edgePos cannot be negative
            if(edgePos < 0)
                throw omnetpp::cRuntimeError("attribute 'edgePos' cannot be negative in element '%s'", speed_tag.c_str());

            // get the lane with maximum length on this edge
            int numLanes = TraCI->edgeGetLaneCount(edgeId_str);
            double maxLaneLength = -1;
            for(int i = 0; i < numLanes; i++)
            {
                std::string laneId = edgeId_str + "_" + std::to_string(i);
                maxLaneLength = std::max(maxLaneLength, TraCI->laneGetLength(laneId));
            }

            if(edgePos > maxLaneLength)
                throw omnetpp::cRuntimeError("attribute 'edgePos' is bigger that the maximum lane length in element '%s'", speed_tag.c_str());
        }

        if(laneId_str != "")
        {
            auto ii = std::find(allLanes.begin(), allLanes.end(), laneId_str);
            if(ii == allLanes.end())
                throw omnetpp::cRuntimeError("Lane '%s' is not a valid lane in element '%s'", laneId_str.c_str(), speed_tag.c_str());

            double laneLength = TraCI->laneGetLength(laneId_str);

            if(lanePos > laneLength)
                throw omnetpp::cRuntimeError("attribute 'lanePos' is bigger than the lane length in element '%s'", speed_tag.c_str());

            if(lanePos < 0)
            {
                lanePos += laneLength;
                if(lanePos < 0)
                    throw omnetpp::cRuntimeError("attribute 'lanePos' is not valid in element '%s'", speed_tag.c_str());
            }
        }

        auto it = allSpeed.find(nodeCount);
        if(it == allSpeed.end())
        {
            speedEntry_t entry = {};

            entry.id_str = id_str;
            entry.begin = begin;
            entry.end = end;
            entry.duration = duration;
            entry.edgeId_str = edgeId_str;
            entry.edgePos = edgePos;
            entry.laneId_str = laneId_str;
            entry.lanePos = lanePos;
            entry.value_str = value_str;
            entry.maxDecel = maxDecel;
            entry.maxAccel = maxAccel;

            // check for conflicts before adding
            checkSpeedConflicts(entry, nodeCount);

            allSpeed.insert(std::make_pair(nodeCount, entry));
        }
        else
            throw omnetpp::cRuntimeError("Multiple %s with the same 'id' %s is not allowed!", speed_tag.c_str(), id_str.c_str());

        // set symbol_table and expression
        auto ii = allSpeed.find(nodeCount);
        ii->second.symbol_table.add_variable("t", ii->second.expressionVariable);
        ii->second.expression.register_symbol_table(ii->second.symbol_table);

        nodeCount++;
    }
}


void TrafficControl::checkSpeedConflicts(speedEntry_t &speedEntry, uint32_t nodeCount)
{
    if(speedEntry.begin != -1)
    {
        uint32_t conflictedSpeedNode = checkSpeedConflicts_begin(speedEntry, nodeCount);
        if(conflictedSpeedNode != 0)
            throw omnetpp::cRuntimeError("Speed node '%d' and '%d' in TrafficControl with id '%s' have conflicts. "
                    "They are both trying to change the speed of vehicle '%s' at time '%f'", conflictedSpeedNode,
                    nodeCount,
                    this->id.c_str(),
                    speedEntry.id_str.c_str(),
                    speedEntry.begin);
    }
    else if(speedEntry.edgeId_str != "")
    {
        uint32_t conflictedSpeedNode = checkSpeedConflicts_edgeId(speedEntry, nodeCount);
        if(conflictedSpeedNode != 0)
            throw omnetpp::cRuntimeError("Speed node '%d' and '%d' in TrafficControl with id '%s' have conflicts. "
                    "They are both trying to change the speed of vehicle '%s' at the same location", conflictedSpeedNode,
                    nodeCount,
                    this->id.c_str(),
                    speedEntry.id_str.c_str());
    }
    else if(speedEntry.laneId_str != "")
    {
        uint32_t conflictedSpeedNode = checkSpeedConflicts_laneId(speedEntry, nodeCount);
        if(conflictedSpeedNode != 0)
            throw omnetpp::cRuntimeError("Speed node '%d' and '%d' in TrafficControl with id '%s' have conflicts. "
                    "They are both trying to change the speed of vehicle '%s' at the same location", conflictedSpeedNode,
                    nodeCount,
                    this->id.c_str(),
                    speedEntry.id_str.c_str());
    }
}


uint32_t TrafficControl::checkSpeedConflicts_begin(speedEntry_t &speedEntry, uint32_t nodeCount)
{
    typedef struct speedChangeEntry
    {
        uint32_t nodeCount;
        double fromTime;
        double toTime;
    } speedChangeEntry_t;

    static std::map<std::string /*vehID*/, std::vector<speedChangeEntry_t>> allSpeedChange;

    double fromTime = speedEntry.begin;
    double toTime = -1;
    if(speedEntry.end != -1)
        toTime = speedEntry.end;
    else if(speedEntry.duration != -1)
        toTime = fromTime + speedEntry.duration;
    else if(speedEntry.end == -1 && speedEntry.duration == -1)
        toTime = fromTime;

    ASSERT(fromTime != -1);
    ASSERT(toTime != -1);

    auto ii = allSpeedChange.find(speedEntry.id_str);
    // this is the first speed control for this vehicle
    if(ii == allSpeedChange.end())
    {
        speedChangeEntry_t entry = {nodeCount, fromTime, toTime};
        std::vector<speedChangeEntry_t> entry2 = {entry};
        allSpeedChange[speedEntry.id_str] = entry2;

        return 0;
    }
    // there is an existing speed control for this vehicle
    else
    {
        // for each node on this lane
        for(auto &speedNode : ii->second)
        {
            // check for overlap
            if((toTime >= speedNode.fromTime && toTime <= speedNode.toTime) ||
                    (fromTime >= speedNode.fromTime && fromTime <= speedNode.toTime))
            {
                return speedNode.nodeCount;
            }
        }

        // there is no overlap!
        speedChangeEntry_t entry = {nodeCount, fromTime, toTime};
        ii->second.push_back(entry);

        return 0;
    }
}


uint32_t TrafficControl::checkSpeedConflicts_edgeId(speedEntry_t &speedEntry, uint32_t nodeCount)
{
    typedef struct speedChangeEntry
    {
        uint32_t nodeCount;
        std::string edgeId;
        double edgePos;
    } speedChangeEntry_t;

    static std::map<std::string /*vehID*/, std::vector<speedChangeEntry_t>> allSpeedChange;

    auto ii = allSpeedChange.find(speedEntry.id_str);
    // this is the first speed control for this vehicle
    if(ii == allSpeedChange.end())
    {
        speedChangeEntry_t entry = {nodeCount, speedEntry.edgeId_str, speedEntry.edgePos};
        std::vector<speedChangeEntry_t> entry2 = {entry};
        allSpeedChange[speedEntry.id_str] = entry2;

        return 0;
    }
    // there is an existing speed control for this vehicle
    else
    {
        // for each node on this lane
        for(auto &speedNode : ii->second)
        {
            // check for overlap
            if(speedEntry.edgeId_str == speedNode.edgeId && speedEntry.edgePos == speedNode.edgePos)
            {
                return speedNode.nodeCount;
            }
        }

        // there is no overlap!
        speedChangeEntry_t entry = {nodeCount, speedEntry.edgeId_str, speedEntry.edgePos};
        ii->second.push_back(entry);

        return 0;
    }
}


uint32_t TrafficControl::checkSpeedConflicts_laneId(speedEntry_t &speedEntry, uint32_t nodeCount)
{
    typedef struct speedChangeEntry
    {
        uint32_t nodeCount;
        std::string laneId;
        double lanePos;
    } speedChangeEntry_t;

    static std::map<std::string /*vehID*/, std::vector<speedChangeEntry_t>> allSpeedChange;

    auto ii = allSpeedChange.find(speedEntry.id_str);
    // this is the first speed control for this vehicle
    if(ii == allSpeedChange.end())
    {
        speedChangeEntry_t entry = {nodeCount, speedEntry.laneId_str, speedEntry.lanePos};
        std::vector<speedChangeEntry_t> entry2 = {entry};
        allSpeedChange[speedEntry.id_str] = entry2;

        return 0;
    }
    // there is an existing speed control for this vehicle
    else
    {
        // for each node on this lane
        for(auto &speedNode : ii->second)
        {
            // check for overlap
            if(speedEntry.laneId_str == speedNode.laneId && speedEntry.lanePos == speedNode.lanePos)
            {
                return speedNode.nodeCount;
            }
        }

        // there is no overlap!
        speedChangeEntry_t entry = {nodeCount, speedEntry.laneId_str, speedEntry.lanePos};
        ii->second.push_back(entry);

        return 0;
    }
}


void TrafficControl::controlSpeed()
{
    for(auto &entry : allSpeed)
    {
        if(!entry.second.processingEnded)
        {
            // begin attribute is present
            if(entry.second.begin != -1)
            {
                // wait until 'begin'
                if(entry.second.begin <= omnetpp::simTime().dbl())
                    controlSpeed_begin(entry.second);
            }
            // edgeId attribute is present
            else if(entry.second.edgeId_str != "")
                controlSpeed_edgeId(entry.second);
            // laneId attribute is present
            else if(entry.second.laneId_str != "")
                controlSpeed_laneId(entry.second);
        }
    }
}


void TrafficControl::controlSpeed_begin(speedEntry_t &speedEntry)
{
    ASSERT(speedEntry.begin != -1);

    // Continuously check if the vehicle is in the network
    auto ii = TraCI->departureArrival.find(speedEntry.id_str);
    if(ii == TraCI->departureArrival.end())
    {
        LOG_WARNING << boost::format("\nWARNING: Changing the speed of '%s' at time '%f' is not possible ") % speedEntry.id_str % speedEntry.begin;
        LOG_WARNING << boost::format("(it hasn't departed yet) \n") << std::flush;
        speedEntry.processingEnded = true;
        return;
    }
    else if(ii->second.arrival != -1)
    {
        if(!speedEntry.processingStarted)
        {
            LOG_WARNING << boost::format("\nWARNING: Changing the speed of '%s' at time '%f' is not possible ") % speedEntry.id_str % speedEntry.begin;
            LOG_WARNING << boost::format("(it left the network at time '%f') \n") % ii->second.arrival << std::flush;
            speedEntry.processingEnded = true;
            return;
        }
        // vehicle arrived while the traffic control is running
        else
        {
            speedEntry.processingEnded = true;
            return;
        }
    }

    if(!speedEntry.processingStarted)
    {
        // save current speed for future
        speedEntry.oldSpeed = TraCI->vehicleGetSpeed(speedEntry.id_str);

        // change the vehicle's speed
        vehicleSetSpeedOnce(speedEntry, speedEntry.value_str);

        speedEntry.processingStarted = true;
    }

    if(speedEntry.expressionEvaluationRequired)
        vehicleSetSpeedExpression(speedEntry, speedEntry.value_str);

    // if neither 'end' or 'duration' is specified
    if(speedEntry.end == -1 && speedEntry.duration == -1)
    {
        if(!speedEntry.expressionEvaluationRequired)
            speedEntry.processingEnded = true;
    }
    // 'end' is specified
    else if(speedEntry.end != -1)
    {
        if(speedEntry.end <= omnetpp::simTime().dbl())
        {
            // revert the vehicle's old speed
            TraCI->vehicleSetSpeed(speedEntry.id_str, speedEntry.oldSpeed);
            speedEntry.processingEnded = true;
        }
    }
    // 'duration' is specified
    else if(speedEntry.duration != -1)
    {
        if(speedEntry.begin + speedEntry.duration <= omnetpp::simTime().dbl())
        {
            // revert the vehicle's old speed
            TraCI->vehicleSetSpeed(speedEntry.id_str, speedEntry.oldSpeed);
            speedEntry.processingEnded = true;
        }
    }
}


void TrafficControl::controlSpeed_edgeId(speedEntry_t &speedEntry)
{
    // Continuously check if the vehicle is in the network
    auto ii = TraCI->departureArrival.find(speedEntry.id_str);
    // the vehicle is not in the network yet!
    if(ii == TraCI->departureArrival.end())
        return;
    else
    {
        if(ii->second.arrival != -1)
        {
            LOG_WARNING << boost::format("\nWARNING: Changing the speed of '%s' at edge '%s' is not possible ") % speedEntry.id_str % speedEntry.edgeId_str;
            LOG_WARNING << boost::format("(it left the network at time '%f') \n") % ii->second.arrival << std::flush;
            speedEntry.processingEnded = true;
            return;
        }
    }

    // now the vehicle is in the network

    // we check (only once) if the vehicle's route contains edgeId_str
    if(!speedEntry.checkRouteEdge)
    {
        auto routeEdges = TraCI->vehicleGetRoute(speedEntry.id_str);
        auto ii = std::find(routeEdges.begin(), routeEdges.end(), speedEntry.edgeId_str);
        if(ii == routeEdges.end())
        {
            LOG_WARNING << boost::format("\nWARNING: Route of vehicle '%s' does not contain edge '%s' \n") % speedEntry.id_str % speedEntry.edgeId_str << std::flush;
            speedEntry.processingEnded = true;
            return;
        }

        speedEntry.checkRouteEdge = true;
    }

    // we check (continuously) the current edge of the vehicle
    if(!speedEntry.onEdge)
    {
        std::string currentEdgeId = TraCI->vehicleGetEdgeID(speedEntry.id_str);
        // the vehicle has not reached to the edgeId yet
        if(currentEdgeId != speedEntry.edgeId_str)
            return;
        else
            speedEntry.onEdge = true;
    }

    // now the vehicle is on the correct edge

    // we check (continuously) the current edge position
    if(!speedEntry.onPos)
    {
        // current position
        double pos = TraCI->vehicleGetLanePosition(speedEntry.id_str);

        // calculated the next position of this vehicle
        double accel = TraCI->vehicleGetCurrentAccel(speedEntry.id_str);
        double speed = TraCI->vehicleGetSpeed(speedEntry.id_str);
        double nextPos = pos + (1/2 * accel * SUMO_timeStep * SUMO_timeStep) + (speed * SUMO_timeStep);

        if(nextPos < speedEntry.edgePos)
            return;
        else
            speedEntry.onPos = true;
    }

    // now the vehicle is on the right edge and right position
    // we let speedControl_begin take over from now
    speedEntry.begin = omnetpp::simTime().dbl();
    controlSpeed_begin(speedEntry);
}


void TrafficControl::controlSpeed_laneId(speedEntry_t &speedEntry)
{
    // Continuously check if the vehicle is in the network
    auto ii = TraCI->departureArrival.find(speedEntry.id_str);
    // the vehicle is not in the network yet!
    if(ii == TraCI->departureArrival.end())
        return;
    else
    {
        if(ii->second.arrival != -1)
        {
            LOG_WARNING << boost::format("\nWARNING: Changing the speed of '%s' at lane '%s' is not possible ") % speedEntry.id_str % speedEntry.laneId_str;
            LOG_WARNING << boost::format("(it left the network at time '%f') \n") % ii->second.arrival << std::flush;
            speedEntry.processingEnded = true;
            return;
        }
    }

    // now the vehicle is in the network

    // we check (only once) if the vehicle's route contains edgeId_str
    if(!speedEntry.checkRouteLane)
    {
        std::string removeTail = speedEntry.laneId_str;
        auto start_position_to_erase = removeTail.find_last_of("_");
        removeTail.erase(start_position_to_erase);

        auto routeEdges = TraCI->vehicleGetRoute(speedEntry.id_str);
        auto ii = std::find(routeEdges.begin(), routeEdges.end(), removeTail);
        if(ii == routeEdges.end())
        {
            LOG_WARNING << boost::format("\nWARNING: Route of vehicle '%s' does not contain lane '%s' \n") % speedEntry.id_str % speedEntry.laneId_str << std::flush;
            speedEntry.processingEnded = true;
            return;
        }

        speedEntry.checkRouteLane = true;
    }

    // we check (continuously) the current lane/pos of the vehicle
    // a vehicle can do lane changing at any time
    // we need to check the lane and pos every time
    if(!speedEntry.onPos)
    {
        std::string currentLaneId = TraCI->vehicleGetLaneID(speedEntry.id_str);
        // the vehicle has not reached to the laneId yet
        if(currentLaneId != speedEntry.laneId_str)
            return;
        else
        {
            // current position
            double pos = TraCI->vehicleGetLanePosition(speedEntry.id_str);

            // calculated the next position of this vehicle
            double accel = TraCI->vehicleGetCurrentAccel(speedEntry.id_str);
            double speed = TraCI->vehicleGetSpeed(speedEntry.id_str);
            double nextPos = pos + (1/2 * accel * SUMO_timeStep * SUMO_timeStep) + (speed * SUMO_timeStep);

            if(nextPos < speedEntry.lanePos)
                return;
            else
                speedEntry.onPos = true;
        }
    }

    // now the vehicle is on the right edge and right position
    // we let speedControl_begin take over from now
    speedEntry.begin = omnetpp::simTime().dbl();
    controlSpeed_begin(speedEntry);
}


void TrafficControl::vehicleSetSpeedOnce(speedEntry_t &speedEntry, std::string speed_str)
{
    if(speedEntry.maxAccel != -1)
        TraCI->vehicleSetMaxAccel(speedEntry.id_str, speedEntry.maxAccel);

    if(speedEntry.maxDecel != -1)
        TraCI->vehicleSetMaxDecel(speedEntry.id_str, speedEntry.maxDecel);

    try
    {
        boost::trim(speed_str);
        double speedValue = boost::lexical_cast<double>(speed_str);

        // make sure the speed is not negative
        ASSERT(speedValue >= 0);

        // set vehicle's speed
        TraCI->vehicleSetSpeed(speedEntry.id_str, speedValue);
    }
    catch(...)
    {
        // we cannot convert speed_str to a double!
        // probably it is an arithmetic expression
        // set the flag
        speedEntry.expressionEvaluationRequired = true;
    }
}


void TrafficControl::vehicleSetSpeedExpression(speedEntry_t &speedEntry, std::string expression_string)
{
    parser_t parser;
    parser.compile(expression_string, speedEntry.expression);

    // evaluate the speed function at current point
    speedEntry.expressionVariable = omnetpp::simTime().dbl();
    double functionVal = speedEntry.expression.value();

    // make sure the speed is not negative
    functionVal = std::max(0., functionVal);

    // set the speed
    TraCI->vehicleSetSpeed(speedEntry.id_str, functionVal);
}


void TrafficControl::parseOptSize(rapidxml::xml_node<> *pNode)
{
    uint32_t nodeCount = 1;

    // Iterate over all 'optSize' nodes
    for(rapidxml::xml_node<> *cNode = pNode->first_node(optSize_tag.c_str()); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != optSize_tag)
            continue;

        std::vector<std::string> validAttr = {"pltId", "begin", "value"};
        xmlUtil::validityCheck(cNode, validAttr);

        std::string pltId_str = xmlUtil::getAttrValue_string(cNode, "pltId", false, "");
        double begin = xmlUtil::getAttrValue_double(cNode, "begin");
        int value = xmlUtil::getAttrValue_int(cNode, "value");

        if(value < 1)
            throw omnetpp::cRuntimeError("attribute 'value' should be >=1 in element '%s'", optSize_tag.c_str());

        if(begin < 0)
            throw omnetpp::cRuntimeError("attribute 'begin' cannot be negative in element '%s'", optSize_tag.c_str());

        auto it = allOptSize.find(nodeCount);
        if(it == allOptSize.end())
        {
            optSizeEntry_t entry = {};

            entry.pltId_str = pltId_str;
            entry.begin = begin;
            entry.value = value;

            allOptSize.insert(std::make_pair(nodeCount, entry));
        }
        else
            throw omnetpp::cRuntimeError("Multiple %s with the same 'id' %s is not allowed!", optSize_tag.c_str(), pltId_str.c_str());

        nodeCount++;
    }
}


void TrafficControl::controlOptSize()
{
    for(auto &entry : allOptSize)
    {
        if(entry.second.processingEnded)
            continue;

        ASSERT(entry.second.begin >= 0);

        // wait until 'begin'
        if(entry.second.begin > omnetpp::simTime().dbl())
            continue;

        if(!entry.second.processingStarted)
        {
            auto allActivePlatoons = TraCI->platoonGetIDList();

            // prepare a list of platoons that their optSize needs to be affected
            std::vector<std::string> affectedPlatoons;
            if(entry.second.pltId_str == "")
                affectedPlatoons = allActivePlatoons;
            else
            {
                // look for the platoon leader
                auto ii = std::find(allActivePlatoons.begin(), allActivePlatoons.end(), entry.second.pltId_str);
                if(ii == allActivePlatoons.end())
                {
                    LOG_WARNING << boost::format("\nWARNING: Changing the optSize of '%s' at time '%f' is not possible ") % entry.second.pltId_str % entry.second.begin;
                    LOG_WARNING << boost::format("(it does not exist in the network) \n") << std::flush;
                    entry.second.processingEnded = true;
                    continue;
                }

                affectedPlatoons.push_back(entry.second.pltId_str);
            }

            // this happens when no pltId is empty and there is no
            // platoon in the network at 'begin'
            if(affectedPlatoons.empty())
            {
                entry.second.processingEnded = true;
                continue;
            }

            for(auto &vehId : affectedPlatoons)
            {
                std::string omnetId = TraCI->convertId_traci2omnet(vehId);

                // get a pointer to the vehicle
                cModule *mod = omnetpp::getSimulation()->getSystemModule()->getModuleByPath(omnetId.c_str());
                ASSERT(mod);

                // get the application module
                cModule *appl = mod->getSubmodule("appl");
                ASSERT(appl);

                // make sure platoon management protocol is 'on'
                if(appl->par("plnMode").longValue() != ApplVPlatoon::platoonManagement)
                {
                    LOG_WARNING << boost::format("\nWARNING: Trying to change optSize in vehicle '%s' with disabled "
                            "platoon management protocol. Is 'pltMgmtProt' attribute active? \n") % vehId << std::flush;
                    entry.second.processingEnded = true;
                    continue;
                }

                // get a pointer to the application layer
                ApplVManager *vehPtr = static_cast<ApplVManager *>(appl);
                ASSERT(vehPtr);

                vehPtr->setOptSize(entry.second.value);
            }

            entry.second.processingStarted = true;
        }

        entry.second.processingEnded = true;
    }
}


void TrafficControl::parsePltMerge(rapidxml::xml_node<> *pNode)
{
    uint32_t nodeCount = 1;

    // Iterate over all 'merge' nodes
    for(rapidxml::xml_node<> *cNode = pNode->first_node(pltMerge_tag.c_str()); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != pltMerge_tag)
            continue;

        std::vector<std::string> validAttr = {"pltId", "begin"};
        xmlUtil::validityCheck(cNode, validAttr);

        std::string pltId_str = xmlUtil::getAttrValue_string(cNode, "pltId");
        double begin = xmlUtil::getAttrValue_double(cNode, "begin");

        if(begin < 0)
            throw omnetpp::cRuntimeError("attribute 'begin' cannot be negative in element '%s'", pltMerge_tag.c_str());

        auto it = allPltMerge.find(nodeCount);
        if(it == allPltMerge.end())
        {
            pltMergeEntry_t entry = {};

            entry.pltId_str = pltId_str;
            entry.begin = begin;

            allPltMerge.insert(std::make_pair(nodeCount, entry));
        }
        else
            throw omnetpp::cRuntimeError("Multiple %s with the same 'id' %s is not allowed!", pltMerge_tag.c_str(), pltId_str.c_str());

        nodeCount++;
    }
}


void TrafficControl::controlPltMerge()
{
    for(auto &entry : allPltMerge)
    {
        if(entry.second.processingEnded)
            continue;

        ASSERT(entry.second.begin >= 0);

        // wait until 'begin'
        if(entry.second.begin > omnetpp::simTime().dbl())
            continue;

        if(!entry.second.processingStarted)
        {
            auto allActivePlatoons = TraCI->platoonGetIDList();

            // look for the platoon leader
            auto ii = std::find(allActivePlatoons.begin(), allActivePlatoons.end(), entry.second.pltId_str);
            if(ii == allActivePlatoons.end())
            {
                LOG_WARNING << boost::format("\nWARNING: Merging platoon '%s' at time '%f' is not possible ") % entry.second.pltId_str % entry.second.begin;
                LOG_WARNING << boost::format("(it does not exist in the network) \n") << std::flush;
                entry.second.processingEnded = true;
                continue;
            }

            std::string omnetId = TraCI->convertId_traci2omnet(entry.second.pltId_str);

            // get a pointer to the vehicle
            cModule *mod = omnetpp::getSimulation()->getSystemModule()->getModuleByPath(omnetId.c_str());
            ASSERT(mod);

            // get the application module
            cModule *appl = mod->getSubmodule("appl");
            ASSERT(appl);

            // make sure platoon management protocol is 'on'
            if(appl->par("plnMode").longValue() != ApplVPlatoon::platoonManagement)
            {
                LOG_WARNING << boost::format("\nWARNING: Trying to perform merge in platoon '%s' with disabled "
                        "platoon management protocol. Is 'pltMgmtProt' attribute active? \n") % entry.second.pltId_str << std::flush;
                entry.second.processingEnded = true;
                continue;
            }

            // make sure there is a vehicle in front of this platoon
            leader_t frontVeh = TraCI->vehicleGetLeader(entry.second.pltId_str, 900);
            if(frontVeh.leaderID == "")
            {
                LOG_WARNING << boost::format("\nWARNING: There is no vehicle in front of platoon '%s' to merge at time '%f'. \n") %
                        entry.second.pltId_str %
                        entry.second.begin << std::flush;
                entry.second.processingEnded = true;
                continue;
            }

            // make sure the front vehicle is part of a platoon
            std::string frontVehId = TraCI->convertId_traci2omnet(frontVeh.leaderID);
            cModule *frontMod = omnetpp::getSimulation()->getSystemModule()->getModuleByPath(frontVehId.c_str());
            ASSERT(frontMod);
            cModule *frontAppl = frontMod->getSubmodule("appl");
            ASSERT(frontAppl);
            if(std::string(frontAppl->par("myPlnID").stringValue()) == "")
            {
                LOG_WARNING << boost::format("\nWARNING: Merge is canceled, because the front vehicle '%s' is not part of any platoon. \n") % frontVeh.leaderID << std::flush;
                entry.second.processingEnded = true;
                continue;
            }

            // get a pointer to the application layer
            ApplVManager *vehPtr = static_cast<ApplVManager *>(appl);
            ASSERT(vehPtr);

            vehPtr->manualMerge();

            entry.second.processingStarted = true;
        }

        entry.second.processingEnded = true;
    }
}


void TrafficControl::parsePltSplit(rapidxml::xml_node<> *pNode)
{
    uint32_t nodeCount = 1;

    // Iterate over all 'split' nodes
    for(rapidxml::xml_node<> *cNode = pNode->first_node(pltSplit_tag.c_str()); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != pltSplit_tag)
            continue;

        std::vector<std::string> validAttr = {"pltId", "splitIndex", "splitVehId", "begin"};
        xmlUtil::validityCheck(cNode, validAttr);

        std::string pltId_str = xmlUtil::getAttrValue_string(cNode, "pltId", false, "");
        int splitIndex = xmlUtil::getAttrValue_int(cNode, "splitIndex", false, -1);
        std::string splitVehId = xmlUtil::getAttrValue_string(cNode, "splitVehId", false, "");
        double begin = xmlUtil::getAttrValue_double(cNode, "begin");

        if(begin < 0)
            throw omnetpp::cRuntimeError("attribute 'begin' cannot be negative in element '%s'", pltSplit_tag.c_str());

        if(cNode->first_attribute("pltId") && !cNode->first_attribute("splitIndex"))
            throw omnetpp::cRuntimeError("attribute 'splitIndex' is required when 'pltId' is present in element '%s'", pltSplit_tag.c_str());

        if(cNode->first_attribute("splitIndex") && !cNode->first_attribute("pltId"))
            throw omnetpp::cRuntimeError("attribute 'pltId' is required when 'splitIndex' is present in element '%s'", pltSplit_tag.c_str());

        if(cNode->first_attribute("splitVehId") && (cNode->first_attribute("pltId") || cNode->first_attribute("splitIndex")))
            throw omnetpp::cRuntimeError("attribute 'pltId/splitIndex' is redundant when 'splitVehId' is present in element '%s'", pltSplit_tag.c_str());

        auto it = allPltSplit.find(nodeCount);
        if(it == allPltSplit.end())
        {
            pltSplitEntry_t entry = {};

            entry.pltId_str = pltId_str;
            entry.splitIndex = splitIndex;
            entry.splitVehId_str = splitVehId;
            entry.begin = begin;

            allPltSplit.insert(std::make_pair(nodeCount, entry));
        }
        else
            throw omnetpp::cRuntimeError("Multiple %s with the same 'id' %s is not allowed!", pltSplit_tag.c_str(), pltId_str.c_str());

        nodeCount++;
    }
}


void TrafficControl::controlPltSplit()
{
    for(auto &entry : allPltSplit)
    {
        if(entry.second.processingEnded)
            continue;

        ASSERT(entry.second.begin >= 0);

        // wait until 'begin'
        if(entry.second.begin > omnetpp::simTime().dbl())
            continue;

        if(!entry.second.processingStarted)
        {
            std::string platoonID_m = "";
            int splitIndex_m = -1;

            // if platoon id is not specified
            if(entry.second.pltId_str == "")
            {
                if(entry.second.splitVehId_str == "")
                    throw omnetpp::cRuntimeError("The 'splitVehId' is empty in element '%s'", pltSplit_tag.c_str());

                std::string omnetId = TraCI->convertId_traci2omnet(entry.second.splitVehId_str);

                if(omnetId == "")
                    throw omnetpp::cRuntimeError("Vehicle '%s' does not exist in the network", entry.second.splitVehId_str.c_str());

                // get a pointer to the splitting vehicle
                cModule *mod = omnetpp::getSimulation()->getSystemModule()->getModuleByPath(omnetId.c_str());
                ASSERT(mod);

                // get the application module
                cModule *appl = mod->getSubmodule("appl");
                ASSERT(appl);

                // get a pointer to the application layer
                ApplVManager *vehPtr = static_cast<ApplVManager *>(appl);
                ASSERT(vehPtr);

                // make sure platoon management protocol is 'on'
                if(appl->par("plnMode").longValue() != ApplVPlatoon::platoonManagement)
                {
                    LOG_WARNING << boost::format("\nWARNING: The platoon management protocol is not active in splitting vehicle '%s'. \n") % entry.second.splitVehId_str << std::flush;
                    entry.second.processingEnded = true;
                    continue;
                }

                // make sure the splitting vehicle is part of a platoon
                if(vehPtr->getPlatoonId() == "")
                {
                    LOG_WARNING << boost::format("\nWARNING: Splitting vehicle '%s' is not part of any platoons. \n") % entry.second.splitVehId_str << std::flush;
                    entry.second.processingEnded = true;
                    continue;
                }

                if(vehPtr->getPlatoonDepth() <= 0)
                {
                    LOG_WARNING << boost::format("\nWARNING: Splitting vehicle '%s' is not a platoon follower. \n") % entry.second.splitVehId_str << std::flush;
                    entry.second.processingEnded = true;
                    continue;
                }

                platoonID_m = vehPtr->getPlatoonId();
                splitIndex_m = vehPtr->getPlatoonDepth();
            }
            else
            {
                platoonID_m = entry.second.pltId_str;
                splitIndex_m = entry.second.splitIndex;
            }

            auto allActivePlatoons = TraCI->platoonGetIDList();

            // look for the platoon leader
            auto ii = std::find(allActivePlatoons.begin(), allActivePlatoons.end(), platoonID_m);
            if(ii == allActivePlatoons.end())
            {
                LOG_WARNING << boost::format("\nWARNING: Platoon splitting in '%s' at time '%f' is not possible ") % platoonID_m % entry.second.begin;
                LOG_WARNING << boost::format("(it does not exist in the network) \n") << std::flush;
                entry.second.processingEnded = true;
                continue;
            }

            std::string omnetId = TraCI->convertId_traci2omnet(platoonID_m);

            // get a pointer to the platoon leader
            cModule *mod = omnetpp::getSimulation()->getSystemModule()->getModuleByPath(omnetId.c_str());
            ASSERT(mod);

            // get the application module
            cModule *appl = mod->getSubmodule("appl");
            ASSERT(appl);

            // make sure platoon management protocol is 'on'
            if(appl->par("plnMode").longValue() != ApplVPlatoon::platoonManagement)
            {
                LOG_WARNING << boost::format("\nWARNING: Trying to perform split in platoon '%s' with disabled "
                        "platoon management protocol. Is 'pltMgmtProt' attribute active? \n") % platoonID_m << std::flush;
                entry.second.processingEnded = true;
                continue;
            }

            // get a pointer to the application layer of the platoon leader
            ApplVManager *vehPtr = static_cast<ApplVManager *>(appl);
            ASSERT(vehPtr);

            int platoonSize = vehPtr->getPlatoonSize();

            if(splitIndex_m <= 0 || splitIndex_m >= platoonSize)
                throw omnetpp::cRuntimeError("The 'splitIndex' value '%d' is invalid in element '%s'", splitIndex_m, pltSplit_tag.c_str());

            vehPtr->splitFromPlatoon(splitIndex_m);

            entry.second.processingStarted = true;
        }

        entry.second.processingEnded = true;
    }
}

}

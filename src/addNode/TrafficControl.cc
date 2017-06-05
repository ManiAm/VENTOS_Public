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
}



void TrafficControl::parseSpeed(rapidxml::xml_node<> *pNode)
{
    uint32_t speedNodeCount = 1;

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

        auto it = allSpeed.find(speedNodeCount);
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
            checkSpeedConflicts(entry, speedNodeCount);

            allSpeed.insert(std::make_pair(speedNodeCount, entry));
        }
        else
            throw omnetpp::cRuntimeError("Multiple %s with the same 'id' %s is not allowed!", speed_tag.c_str(), id_str.c_str());

        speedNodeCount++;
    }
}


void TrafficControl::checkSpeedConflicts(speedEntry_t &speedEntry, uint32_t speedNodeCount)
{
    if(speedEntry.begin != -1)
    {
        uint32_t conflictedSpeedNode = checkSpeedConflicts_begin(speedEntry, speedNodeCount);
        if(conflictedSpeedNode != 0)
            throw omnetpp::cRuntimeError("Speed node '%d' and '%d' in TrafficControl with id '%s' have conflicts. "
                    "They are both trying to change the speed of vehicle '%s' at time '%f'", conflictedSpeedNode,
                    speedNodeCount,
                    this->id.c_str(),
                    speedEntry.id_str.c_str(),
                    speedEntry.begin);
    }
    else if(speedEntry.edgeId_str != "")
    {
        uint32_t conflictedSpeedNode = checkSpeedConflicts_edgeId(speedEntry, speedNodeCount);
        if(conflictedSpeedNode != 0)
            throw omnetpp::cRuntimeError("Speed node '%d' and '%d' in TrafficControl with id '%s' have conflicts. "
                    "They are both trying to change the speed of vehicle '%s' at the same location", conflictedSpeedNode,
                    speedNodeCount,
                    this->id.c_str(),
                    speedEntry.id_str.c_str());
    }
    else if(speedEntry.laneId_str != "")
    {
        uint32_t conflictedSpeedNode = checkSpeedConflicts_laneId(speedEntry, speedNodeCount);
        if(conflictedSpeedNode != 0)
            throw omnetpp::cRuntimeError("Speed node '%d' and '%d' in TrafficControl with id '%s' have conflicts. "
                    "They are both trying to change the speed of vehicle '%s' at the same location", conflictedSpeedNode,
                    speedNodeCount,
                    this->id.c_str(),
                    speedEntry.id_str.c_str());
    }
}


uint32_t TrafficControl::checkSpeedConflicts_begin(speedEntry_t &speedEntry, uint32_t speedNodeCount)
{
    typedef struct speedChangeEntry
    {
        uint32_t speedNodeCount;
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
        speedChangeEntry_t entry = {speedNodeCount, fromTime, toTime};
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
                return speedNode.speedNodeCount;
            }
        }

        // there is no overlap!
        speedChangeEntry_t entry = {speedNodeCount, fromTime, toTime};
        ii->second.push_back(entry);

        return 0;
    }
}


uint32_t TrafficControl::checkSpeedConflicts_edgeId(speedEntry_t &speedEntry, uint32_t speedNodeCount)
{
    typedef struct speedChangeEntry
    {
        uint32_t speedNodeCount;
        std::string edgeId;
        double edgePos;
    } speedChangeEntry_t;

    static std::map<std::string /*vehID*/, std::vector<speedChangeEntry_t>> allSpeedChange;

    auto ii = allSpeedChange.find(speedEntry.id_str);
    // this is the first speed control for this vehicle
    if(ii == allSpeedChange.end())
    {
        speedChangeEntry_t entry = {speedNodeCount, speedEntry.edgeId_str, speedEntry.edgePos};
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
                return speedNode.speedNodeCount;
            }
        }

        // there is no overlap!
        speedChangeEntry_t entry = {speedNodeCount, speedEntry.edgeId_str, speedEntry.edgePos};
        ii->second.push_back(entry);

        return 0;
    }
}


uint32_t TrafficControl::checkSpeedConflicts_laneId(speedEntry_t &speedEntry, uint32_t speedNodeCount)
{
    typedef struct speedChangeEntry
    {
        uint32_t speedNodeCount;
        std::string laneId;
        double lanePos;
    } speedChangeEntry_t;

    static std::map<std::string /*vehID*/, std::vector<speedChangeEntry_t>> allSpeedChange;

    auto ii = allSpeedChange.find(speedEntry.id_str);
    // this is the first speed control for this vehicle
    if(ii == allSpeedChange.end())
    {
        speedChangeEntry_t entry = {speedNodeCount, speedEntry.laneId_str, speedEntry.lanePos};
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
                return speedNode.speedNodeCount;
            }
        }

        // there is no overlap!
        speedChangeEntry_t entry = {speedNodeCount, speedEntry.laneId_str, speedEntry.lanePos};
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


void TrafficControl::vehicleSetSpeedExpression(speedEntry_t &speedEntry, std::string expression)
{

    TraCI->vehicleSetSpeed(speedEntry.id_str, 5.);

}


void TrafficControl::parseOptSize(rapidxml::xml_node<> *pNode)
{

}


void TrafficControl::controlOptSize()
{

}


void TrafficControl::parsePltMerge(rapidxml::xml_node<> *pNode)
{

}


void TrafficControl::controlPltMerge()
{

}


void TrafficControl::parsePltSplit(rapidxml::xml_node<> *pNode)
{

}


void TrafficControl::controlPltSplit()
{

}

}

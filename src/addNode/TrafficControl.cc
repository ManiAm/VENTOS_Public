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

    // Iterate over all 'speed' nodes
    for(rapidxml::xml_node<> *cNode = pNode->first_node(speed_tag.c_str()); cNode; cNode = cNode->next_sibling())
    {
        if(std::string(cNode->name()) != speed_tag)
            continue;

        std::vector<std::string> validAttr = {"id", "begin", "end", "duration", "edgeId", "edgePos", "laneId",
                "lanePos", "value", "maxAccel", "maxDecel"};
        xmlUtil::validityCheck(cNode, validAttr);

        std::string id_str = xmlUtil::getAttrValue_string(cNode, "id");
        double begin = xmlUtil::getAttrValue_double(cNode, "begin");
        double end = xmlUtil::getAttrValue_double(cNode, "end", false, -1);
        double duration = xmlUtil::getAttrValue_double(cNode, "duration", false, -1);
        std::string edgeId_str = xmlUtil::getAttrValue_string(cNode, "edgeId", false, "");
        double edgePos = xmlUtil::getAttrValue_double(cNode, "edgePos", false, -1);
        std::string laneId_str = xmlUtil::getAttrValue_string(cNode, "laneId", false, "");
        double lanePos = xmlUtil::getAttrValue_double(cNode, "lanePos", false, -1);
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

            entry.processingStarted = false;
            entry.processingEnded = false;
            entry.oldSpeed = -1;

            allSpeed.insert(std::make_pair(speedNodeCount, entry));
        }
        else
            throw omnetpp::cRuntimeError("Multiple %s with the same 'id' %s is not allowed!", speed_tag.c_str(), id_str.c_str());

        // todo: check for conflict.
        // we can not apply two different speed at the same time

        speedNodeCount++;
    }
}


void TrafficControl::controlSpeed()
{
    for(auto &entry : allSpeed)
    {
        if(!entry.second.processingEnded)
        {
            // edgeId attribute is present
            if(entry.second.edgeId_str != "")
                controlSpeed_edgeId(entry.second);
            // laneId attribute is present
            else if(entry.second.laneId_str != "")
                controlSpeed_laneId(entry.second);
            // begin attribute is present
            else
                controlSpeed_begin(entry.second);
        }
    }
}


void TrafficControl::controlSpeed_begin(speedEntry_t &speedEntry)
{
    // we need to wait for the vehicle to be inserted
    if(speedEntry.begin == -1)
    {
        auto ii = TraCI->departureArrival.find(speedEntry.id_str);
        if(ii == TraCI->departureArrival.end())
            return;
        else
        {
            if(ii->second.arrival != -1)
            {
                LOG_WARNING << boost::format("\nWARNING: Vehicle '%s' has already left the network at '%f'") % speedEntry.id_str % ii->second.arrival << std::flush;
                speedEntry.processingEnded = true;
                return;
            }
            else
            {
                speedEntry.begin = omnetpp::simTime().dbl();
                // now begin has value and we fall to the next section
            }
        }
    }

    if(speedEntry.begin != -1 && speedEntry.begin <= omnetpp::simTime().dbl())
    {
        // Continuously check if the vehicle is in the network
        auto ii = TraCI->departureArrival.find(speedEntry.id_str);
        if(ii == TraCI->departureArrival.end())
        {
            // vehicle does not exist in the 'begin' time. Show a warning an terminate
            LOG_WARNING << boost::format("\nWARNING: Vehicle '%s' does not exist at time '%f'") % speedEntry.id_str % speedEntry.begin << std::flush;
            speedEntry.processingEnded = true;
            return;
        }
        else if(ii->second.arrival != -1)
        {
            // vehicle has already left the network at the 'begin'
            if(!speedEntry.processingStarted)
            {
                LOG_WARNING << boost::format("\nWARNING: Vehicle '%s' has already left the network at '%f'") % speedEntry.id_str % ii->second.arrival << std::flush;
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
            // save current params for future
            speedEntry.oldSpeed = TraCI->vehicleGetSpeed(speedEntry.id_str);

            // change the vehicle's speed
            vehicleSetSpeed(speedEntry, speedEntry.value_str);

            speedEntry.processingStarted = true;
        }

        // if neither 'end' or 'duration' is specified
        if(speedEntry.end == -1 && speedEntry.duration == -1)
        {
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
}


void TrafficControl::controlSpeed_edgeId(speedEntry_t &speedEntry)
{

}


void TrafficControl::controlSpeed_laneId(speedEntry_t &speedEntry)
{

}


void TrafficControl::vehicleSetSpeed(speedEntry_t &speedEntry, std::string speed_str)
{
    try
    {
        // convert string to double
        std::string::size_type sz;  // alias of size_t
        double speedValue = std::stod(speed_str, &sz);

        // make sure the speed is not negative
        ASSERT(speedValue >= 0);

        // set vehicle's speed
        TraCI->vehicleSetSpeed(speedEntry.id_str, speedValue);

        if(speedEntry.maxAccel != -1)
            TraCI->vehicleSetMaxAccel(speedEntry.id_str, speedEntry.maxAccel);

        if(speedEntry.maxDecel != -1)
            TraCI->vehicleSetMaxDecel(speedEntry.id_str, speedEntry.maxDecel);
    }
    catch(...)
    {
        throw omnetpp::cRuntimeError("The speed value is invalid in element '%s': %s", speed_tag.c_str(), speed_str.c_str());
    }
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

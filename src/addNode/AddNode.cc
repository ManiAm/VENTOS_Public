/****************************************************************************/
/// @file    AddNode.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    Apr 2016
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

#include "AddNode.h"
#include "ConnectionManager.h"

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>

// un-defining ev!
// why? http://stackoverflow.com/questions/24103469/cant-include-the-boost-filesystem-header
#undef ev
#include "boost/filesystem.hpp"
#define ev  (*cSimulation::getActiveEnvir())

namespace VENTOS {

Define_Module(VENTOS::AddNode);

AddNode::~AddNode()
{

}


void AddNode::initialize(int stage)
{
    if(stage ==0)
    {
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        terminate = module->par("terminate").doubleValue();
        // if user specifies no termination time, set it to a big value
        if(terminate == -1)
            terminate = 100000;

        Signal_addFlow = registerSignal("addFlow");
        simulation.getSystemModule()->subscribe("addFlow", this);
    }
}


void AddNode::finish()
{
    cModule *module = simulation.getSystemModule()->getSubmodule("connMan");
    ConnectionManager *cc = static_cast<ConnectionManager*>(module);
    ASSERT(cc);

    // delete all RSU modules in omnet
    for(auto i : RSUhosts)
    {
        cModule* mod = i.second;
        cc->unregisterNic(mod->getSubmodule("nic"));

        mod->callFinish();
        mod->deleteModule();
    }
}


void AddNode::handleMessage(cMessage *msg)
{

}


void AddNode::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_addFlow)
    {
        addFlow();
    }
}


// ask SUMO to add a vehicle
void AddNode::addVehicle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, int32_t depart, double pos, double speed, uint8_t lane)
{
    TraCI->vehicleAdd(vehicleId, vehicleTypeId, routeId, depart, pos, speed, lane);
}


// ask SUMO to add a bicycle
void AddNode::addBicycle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, int32_t depart, double pos, double speed, uint8_t lane)
{
    TraCI->vehicleAdd(vehicleId, vehicleTypeId, routeId, depart, pos, speed, lane);
}


// add adversary module to OMNET
void AddNode::addAdversary(int num)
{
    if(num <= 0)
        error("num should be > 0");

    cModule* parentMod = getParentModule();
    if (!parentMod)
        error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get("VENTOS.src.adversary.Adversary");

    for(int i = 0; i < num; i++)
    {
        // do not use create("adversary", parentMod);
        // instead create an array of adversaries
        cModule* mod = nodeType->create("adversary", parentMod, num, i);
        mod->finalizeParameters();
        mod->getDisplayString().updateWith("i=old/comp_a");
        mod->buildInside();
        mod->scheduleStart(simTime());
        mod->callInitialize();
    }
}


// add Certificate Authority (CA) module to OMNET
void AddNode::addCA(int num)
{
    if(num <= 0)
        error("num should be > 0");

    cModule* parentMod = getParentModule();
    if (!parentMod)
        error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get("VENTOS.src.CerAuthority.CA");

    for(int i = 0; i < num; i++)
    {
        // do not use create("CA", parentMod);
        // instead create an array of adversaries
        cModule* mod = nodeType->create("CA", parentMod, num, i);
        mod->finalizeParameters();
        mod->getDisplayString().updateWith("i=old/comp_a");
        mod->buildInside();
        mod->scheduleStart(simTime());
        mod->callInitialize();
    }
}


// add RSU modules to OMNET/SUMO
void AddNode::addRSU(int num)
{
    if(num <= 0)
        error("num should be > 0");

    // ######################
    // create RSUs in OMNET++
    // ######################

    cModule* parentMod = getParentModule();
    if (!parentMod)
        error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get("VENTOS.src.rsu.RSU");

    std::list<std::string> TLList = TraCI->TLGetIDList();

    // creating RSU modules in OMNET (without moving them to the correct position)
    for(int i = 0; i < num; i++)
    {
        cModule* mod = nodeType->create("RSU", parentMod, num, i);

        mod->finalizeParameters();
        mod->getDisplayString().updateWith("i=device/antennatower");
        mod->buildInside();

        std::string RSUname = mod->getSubmodule("appl")->par("SUMOID").stringValue();

        // check if any TLid is associated with this RSU
        std::string myTLid = "";
        for(std::string TLid : TLList)
        {
            if(TLid == RSUname)
            {
                myTLid = TLid;
                break;
            }
        }

        // then set the myTLid parameter
        mod->getSubmodule("appl")->par("myTLid") = myTLid;

        mod->scheduleStart(simTime());
        mod->callInitialize();

        // store the cModule of this RSU
        RSUhosts[i] = mod;
    }

    // ###############################################################
    // draw RSUs in SUMO (using a circle to show radio coverage)
    // ###############################################################

    for(int i = 0; i < num; i++)
    {
        // get a reference to this RSU
        cModule *module = simulation.getSystemModule()->getSubmodule("RSU", i);

        // get SUMOID
        std::string RSUname = module->getSubmodule("appl")->par("SUMOID").stringValue();

        // get the radius of this RSU
        double radius = atof( module->getDisplayString().getTagArg("r",0) );

        // get SUMO X and Y
        double X = module->getSubmodule("mobility")->par("x").doubleValue();
        double Y = module->getSubmodule("mobility")->par("y").doubleValue();

        Coord *center = new Coord(X,Y);
        commandAddCirclePoly(RSUname, "RSU", Color::colorNameToRGB("green"), center, radius);
    }
}


void AddNode::commandAddCirclePoly(std::string name, std::string type, const RGB color, Coord *center, double radius)
{
    std::list<TraCICoord> circlePoints;

    // Convert from degrees to radians via multiplication by PI/180
    for(int angleInDegrees = 0; angleInDegrees <= 360; angleInDegrees = angleInDegrees + 10)
    {
        double x = (double)( radius * cos(angleInDegrees * 3.14 / 180) ) + center->x;
        double y = (double)( radius * sin(angleInDegrees * 3.14 / 180) ) + center->y;

        circlePoints.push_back(TraCICoord(x, y));
    }

    // create polygon in SUMO
    TraCI->polygonAddTraCI(name, type, color, 0, 1, circlePoints);
}


void AddNode::addFlow()
{
    // get full path to the sumo.cfg file
    std::string sumoConfig = TraCI->getSUMOConfigFullPath();

    // read sumo.cfg file and get the path to rou file
    std::string sumoRou = getFullPathToSumoRou(sumoConfig);

    // read flows.xml file and extract a flow set
    //     getFlowSet();

    // copy the flow set into a new rou file in %TMP%
    //     addFlowSetToNewRou();

    // add a new entry to copy the new rou, and also modify sumo.cfg
    //    applyChanges();
}


std::string AddNode::getFullPathToSumoRou(std::string sumoConfigFullPath)
{

    return "";

}

}


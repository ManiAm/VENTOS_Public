/****************************************************************************/
/// @file    AddFixNode.cc
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

#include "AddFixNode.h"
#include "ConnectionManager.h"
#include <vlog.h>

#undef ev
#include "boost/filesystem.hpp"

namespace VENTOS {

Define_Module(VENTOS::AddFixNode);

AddFixNode::~AddFixNode()
{

}


void AddFixNode::initialize(int stage)
{
    super::initialize(stage);

    if(stage ==0)
    {
        // get a pointer to the TraCI module
        omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initialize_withTraCI", this);
    }
}


void AddFixNode::finish()
{
    omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("connMan");
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

    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("initialize_withTraCI", this);
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("executeEachTS", this);
}


void AddFixNode::handleMessage(omnetpp::cMessage *msg)
{

}


void AddFixNode::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        beginLoading();
    }
}


void AddFixNode::beginLoading()
{
    int numRSUs = par("numRSUs").longValue();
    int numCA = par("numCA").longValue();
    int numAdversary = par("numAdversary").longValue();

    if(numRSUs > 0 || numCA > 0 || numAdversary > 0)
        LOG_DEBUG << "\n>>> AddFixNode is adding nodes to the simulation ... \n" << std::flush;

    if(numRSUs > 0)
        addRSU(numRSUs);

    if(numCA > 0)
        addCA(numCA);

    if(numAdversary > 0)
        addAdversary(numAdversary);

    if( LOG_ACTIVE(DEBUG_LOG_VAL) & (numRSUs > 0 || numCA > 0 || numAdversary > 0) )
    {
        LOG_DEBUG << "\n>>> AddFixNode is done adding nodes. Here is a summary: \n" << std::flush;
        printLoadedStatistics();
    }
}


void AddFixNode::printLoadedStatistics()
{
    // get a pointer to the first RSU
    omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("RSU", 0);
    if(module != NULL)
    {
        // how many RSUs are in the network?
        int RSUcount = module->getVectorSize();
        LOG_DEBUG << boost::format("  %1% RSU modules are added: ") % RSUcount;

        // iterate over modules
        for(int i = 0; i < RSUcount; ++i)
        {
            // get a pointer to the RSU
            module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("RSU", i);

            // get OMNET id
            std::string fullId = module->getFullName();

            // get SUMO id
            cModule *appl =  module->getSubmodule("appl");
            std::string SUMOID = appl->par("SUMOID").stringValue();

            LOG_DEBUG << boost::format("%1% %2%, ") % fullId % SUMOID;
        }

        LOG_DEBUG << "\n";
    }

    // get a pointer to the first Adversary
    module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("adversary", 0);
    if(module != NULL)
    {
        // how many Adversaries are in the network?
        int advCount = module->getVectorSize();
        LOG_DEBUG << boost::format("  %1% adversary modules are added: ") % advCount;

        // iterate over modules
        for(int i = 0; i < advCount; ++i)
        {
            // get a pointer to the RSU
            module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("adversary", i);

            // get OMNET id
            std::string fullId = module->getFullName();

            printf("%s, ", fullId.c_str());
        }

        LOG_DEBUG << "\n";
    }

    // get a pointer to the first CA
    module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("CA", 0);
    if(module != NULL)
    {
        // how many Adversaries are in the network?
        int CACount = module->getVectorSize();
        LOG_DEBUG << boost::format("  %1% CA modules are added: ") % CACount;

        // iterate over modules
        for(int i = 0; i < CACount; ++i)
        {
            // get a pointer to the CA
            module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("CA", i);

            // get OMNET id
            std::string fullId = module->getFullName();

            LOG_DEBUG << boost::format("%s, ") % fullId;
        }

        LOG_DEBUG << "\n";
    }

    LOG_FLUSH;
}


// add adversary modules to OMNET (without moving them to the correct position)
void AddFixNode::addAdversary(int num)
{
    if(num <= 0)
        throw omnetpp::cRuntimeError("num should be > 0");

    cModule* parentMod = getParentModule();
    if (!parentMod)
        throw omnetpp::cRuntimeError("Parent Module not found");

    omnetpp::cModuleType* nodeType = omnetpp::cModuleType::get("VENTOS.src.adversary.Adversary");

    for(int i = 0; i < num; i++)
    {
        // do not use create("adversary", parentMod);
        // instead create an array of adversaries
        cModule* mod = nodeType->create("adversary", parentMod, num, i);
        mod->finalizeParameters();
        mod->getDisplayString().updateWith("i=old/comp_a");
        mod->buildInside();
        mod->scheduleStart(omnetpp::simTime());
        mod->callInitialize();
    }

    // now we draw adversary modules in SUMO (using a circle to show radio coverage)
    for(int i = 0; i < num; i++)
    {
        // get a reference to this adversary
        omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("adversary", i);
        ASSERT(module);

        // get ID
        std::string name = module->getFullName();
        // get the radius of this RSU
        double radius = atof( module->getDisplayString().getTagArg("r",0) );

        // get SUMO X and Y
        double X = module->getSubmodule("mobility")->par("x").doubleValue();
        double Y = module->getSubmodule("mobility")->par("y").doubleValue();

        Coord *center = new Coord(X,Y);
        addCircle(name, "Adv", Color::colorNameToRGB("green"), 1, center, radius);
    }
}


// add Certificate Authority (CA) modules to OMNET (without moving them to the correct position)
void AddFixNode::addCA(int num)
{
    if(num <= 0)
        throw omnetpp::cRuntimeError("num should be > 0");

    cModule* parentMod = getParentModule();
    if (!parentMod)
        throw omnetpp::cRuntimeError("Parent Module not found");

    omnetpp::cModuleType* nodeType = omnetpp::cModuleType::get("VENTOS.src.CerAuthority.CA");

    for(int i = 0; i < num; i++)
    {
        // do not use create("CA", parentMod);
        // instead create an array of adversaries
        cModule* mod = nodeType->create("CA", parentMod, num, i);
        mod->finalizeParameters();
        mod->getDisplayString().updateWith("i=old/comp_a");
        mod->buildInside();
        mod->scheduleStart(omnetpp::simTime());
        mod->callInitialize();
    }
}


// add RSU modules to OMNET/SUMO (without moving them to the correct position)
void AddFixNode::addRSU(int num)
{
    if(num <= 0)
        throw omnetpp::cRuntimeError("num should be > 0");

    cModule* parentMod = getParentModule();
    if (!parentMod)
        throw omnetpp::cRuntimeError("Parent Module not found");

    omnetpp::cModuleType* nodeType = omnetpp::cModuleType::get("VENTOS.src.rsu.RSU");

    std::list<std::string> TLList = TraCI->TLGetIDList();

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

        mod->scheduleStart(omnetpp::simTime());
        mod->callInitialize();

        // store the cModule of this RSU
        RSUhosts[i] = mod;
    }


    // now we draw RSUs in SUMO (using a circle to show radio coverage)
    for(int i = 0; i < num; i++)
    {
        // get a reference to this RSU
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("RSU", i);
        ASSERT(module);

        // get SUMOID
        std::string name = module->getSubmodule("appl")->par("SUMOID").stringValue();

        // get the radius of this RSU
        double radius = atof( module->getDisplayString().getTagArg("r",0) );

        // get SUMO X and Y
        double X = module->getSubmodule("mobility")->par("x").doubleValue();
        double Y = module->getSubmodule("mobility")->par("y").doubleValue();

        Coord *center = new Coord(X,Y);
        addCircle(name, "RSU", Color::colorNameToRGB("green"), 0, center, radius);
    }
}


void AddFixNode::addCircle(std::string name, std::string type, const RGB color, bool filled, Coord *center, double radius)
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
    TraCI->polygonAddTraCI(name, type, color, filled /*filled*/, 1 /*layer*/, circlePoints);
}

}


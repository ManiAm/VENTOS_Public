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


void AddNode::addAdversary()
{
    cModule* parentMod = getParentModule();
    if (!parentMod)
        error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get("c3po.application.adversary.Adversary");

    // do not use create("adversary", parentMod);
    // instead create an array of adversaries
    cModule* mod = nodeType->create("adversary", parentMod, 1, 0);
    mod->finalizeParameters();
    mod->getDisplayString().updateWith("i=old/comp_a");
    mod->buildInside();
    mod->scheduleStart(simTime());
    mod->callInitialize();
}


void AddNode::addvehicle()
{


}


void AddNode::addBicycle()
{


}


void AddNode::addRSU()
{
    // ####################################
    // Step 1: read RSU locations from file
    // ####################################

    std::string RSUfile = par("RSUfile").stringValue();
    boost::filesystem::path dir (TraCI->getSUMOFullDir());
    boost::filesystem::path RSUfilePath = dir / RSUfile;

    // check if this file is valid?
    if( !boost::filesystem::exists(RSUfilePath) )
        error("RSU file does not exist in %s", RSUfilePath.string().c_str());

    std::map<std::string, RSUEntry> RSUs = commandReadRSUsCoord(RSUfilePath.string());

    // ##############################
    // Step 2: create RSUs in OMNET++
    // ##############################

    cModule* parentMod = getParentModule();
    if (!parentMod)
        error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get("VENTOS.src.rsu.RSU");

    std::list<std::string> TLList = TraCI->TLGetIDList();

    // creating RSU modules in OMNET (without moving them to the correct position)
    int count = 0;
    for(auto &z : RSUs)
    {
        std::string RSUname = z.first;
        RSUEntry entry = z.second;

        cModule* mod = nodeType->create("RSU", parentMod, RSUs.size(), count);

        mod->finalizeParameters();
        mod->getDisplayString().updateWith("i=device/antennatower");
        mod->buildInside();

        // check if any TLid is associated with this RSU
        std::string myTLid = "";
        for(auto &y : TLList)
        {
            std::string TLid = y;
            if( TLid == RSUname )
            {
                myTLid = TLid;
                break;
            }
        }
        // then set the myTLid parameter
        mod->getSubmodule("appl")->par("myTLid") = myTLid;

        mod->getSubmodule("appl")->par("SUMOID") = RSUname;

        // set coordinates (RSUMobility uses these to move the RSU to the correct location)
        mod->getSubmodule("appl")->par("myCoordX") = entry.coordX;
        mod->getSubmodule("appl")->par("myCoordY") = entry.coordY;

        mod->scheduleStart(simTime());
        mod->callInitialize();

        // store the cModule of this RSU
        RSUhosts[count] = mod;

        count++;
    }

    // ###############################################################
    // Step 3: draw RSU in SUMO (using a circle to show radio coverage)
    // ###############################################################

    count = 0;
    for(auto &z : RSUs)
    {
        std::string RSUname = z.first;
        RSUEntry entry = z.second;

        // get the radius of this RSU
        cModule *module = simulation.getSystemModule()->getSubmodule("RSU", count);
        double radius = atof( module->getDisplayString().getTagArg("r",0) );

        Coord *center = new Coord(entry.coordX, entry.coordY);
        commandAddCirclePoly(RSUname, "RSU", Color::colorNameToRGB("green"), center, radius);

        count++;
    }
}


std::map<std::string, RSUEntry> AddNode::commandReadRSUsCoord(std::string RSUfilePath)
{
    rapidxml::file<> xmlFile( RSUfilePath.c_str() );        // Convert our file to a rapid-xml readable object
    rapidxml::xml_document<> doc;                           // Build a rapidxml doc
    doc.parse<0>(xmlFile.data());                 // Fill it with data from our file
    rapidxml::xml_node<> *node = doc.first_node("RSUs");    // Parse up to the "RSUs" declaration

    std::map<std::string, RSUEntry> RSUs;

    for(node = node->first_node("poly"); node; node = node->next_sibling())
    {
        std::string RSUname = "";
        std::string RSUtype = "";
        std::string RSUcoordinates = "";
        int readCount = 1;

        // For each node, iterate over its attributes until we reach "shape"
        for(rapidxml::xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
        {
            if(readCount == 1)
            {
                RSUname = attr->value();
            }
            else if(readCount == 2)
            {
                RSUtype = attr->value();

                if(RSUtype != "RSU")
                    error("RSU type should be 'RSU'");
            }
            else if(readCount == 3)
            {
                RSUcoordinates = attr->value();
            }

            readCount++;
        }

        if(readCount != 4)
            error("format of RSUsLocation.xml file is wrong!");

        std::vector<double> vec = cStringTokenizer(RSUcoordinates.c_str(), ",").asDoubleVector();
        if(vec.size() != 2)
            error("center coordinate format is wrong!");

        // add it into queue (with TraCI coordinates)
        RSUEntry *entry = new RSUEntry(RSUtype, vec[0], vec[1]);
        RSUs.insert( std::make_pair(RSUname, *entry) );
    }

    return RSUs;
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


void AddNode::addCA()
{
    cModule* parentMod = getParentModule();
    if (!parentMod)
        error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get("VENTOS.src.CerAuthority.CA");

    // do not use create("CA", parentMod);
    // instead create an array of adversaries
    cModule* mod = nodeType->create("CA", parentMod, 1, 0);
    mod->finalizeParameters();
    mod->getDisplayString().updateWith("i=old/comp_a");
    mod->buildInside();
    mod->scheduleStart(simTime());
    mod->callInitialize();
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


void AddNode::printLoadedStatistics()
{
    // ######################
    // list all entities type
    //#######################

    std::list<std::string> loadedVehList = TraCI->simulationGetLoadedVehiclesIDList();
    std::cout << ">>> AddScenario module loaded " << loadedVehList.size() << " entities: " << endl;
    std::cout << endl;

    {
        std::list<std::string> loadedVehTypeList = TraCI->vehicleTypeGetIDList();
        std::cout << "  " << loadedVehTypeList.size() << " possible vehicle types are loaded: " << endl;
        std::cout << "      ";
        unsigned int counter = 1;
        for(std::string type : loadedVehTypeList)
        {
            std::cout << type << "  ";

            // introduce new line after each 4 routes
            if(counter % 4 == 0 && counter != loadedVehTypeList.size())
                std::cout << std::endl << "      ";

            counter++;
        }

        std::cout << std::endl;
        std::cout << std::endl;

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
            std::cout << "  ";
            std::cout << count << " entities are loaded of type " << "\"" << type << "\"" << endl;
        }
    }

    //###########################
    // list all the loaded routes
    //###########################

    {
        std::list<std::string> loadedRouteList = TraCI->routeGetIDList();
        std::cout << std::endl;
        std::cout << "  " << loadedRouteList.size() << " possible routes are loaded: " << endl;
        std::cout << "      ";
        unsigned int counter = 1;
        for(std::string route : loadedRouteList)
        {
            std::cout << route << "  ";

            // introduce new line after each 4 routes
            if(counter % 4 == 0 && counter != loadedRouteList.size())
                std::cout << std::endl << "      ";

            counter++;
        }

        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "  route distributions are: " << endl;

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
            std::cout << "      ";
            std::cout << count << " entities have route " << "\"" << route << "\"" << endl;
        }
    }
}

}


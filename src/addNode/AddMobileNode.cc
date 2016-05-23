/****************************************************************************/
/// @file    AddMobileNode.cc
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

#include "AddMobileNode.h"
#include "Router.h"
#include <algorithm>
#include <random>
#include <vlog.h>

namespace VENTOS {

Define_Module(VENTOS::AddMobileNode);

AddMobileNode::~AddMobileNode()
{

}


void AddMobileNode::initialize(int stage)
{
    super::initialize(stage);

    if(stage ==0)
    {
        mode = par("mode").longValue();

        // get a pointer to the TraCI module
        omnetpp::cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Commands *>(module);
        ASSERT(TraCI);

        terminate = TraCI->par("terminate").doubleValue();
        // if user specifies no termination time, set it to a big value
        if(terminate == -1)
            terminate = 10000;

        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initialize_withTraCI", this);

        Signal_addFlow = registerSignal("addFlow");
        omnetpp::getSimulation()->getSystemModule()->subscribe("addFlow", this);
    }
}


void AddMobileNode::finish()
{
    // unsubscribe
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("initialize_withTraCI", this);
    omnetpp::getSimulation()->getSystemModule()->unsubscribe("executeEachTS", this);
}


void AddMobileNode::handleMessage(omnetpp::cMessage *msg)
{

}


void AddMobileNode::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    if(mode <= -1)
        return;

    Enter_Method_Silent();

    if(signalID == Signal_initialize_withTraCI)
    {
        beginLoading();
    }
    else if(signalID == Signal_addFlow)
    {
        addFlow();
    }
}


void AddMobileNode::beginLoading()
{
    LOG_DEBUG << "\n>>> AddMobileNode is adding nodes to the simulation ... \n" << std::flush;

    // create a map of functions
    typedef void (AddMobileNode::*pfunc)(void);
    std::map<std::string, pfunc> funcMap;
    funcMap["Scenario1"] = &AddMobileNode::Scenario1;
    funcMap["Scenario2"] = &AddMobileNode::Scenario2;
    funcMap["Scenario4"] = &AddMobileNode::Scenario4;
    funcMap["Scenario5"] = &AddMobileNode::Scenario5;
    funcMap["Scenario6"] = &AddMobileNode::Scenario6;
    funcMap["Scenario7"] = &AddMobileNode::Scenario7;
    funcMap["Scenario8"] = &AddMobileNode::Scenario8;
    funcMap["Scenario9"] = &AddMobileNode::Scenario9;
    funcMap["Scenario10"] = &AddMobileNode::Scenario10;
    funcMap["Scenario11"] = &AddMobileNode::Scenario11;
    funcMap["Scenario12"] = &AddMobileNode::Scenario12;

    // construct the method name
    std::ostringstream out;
    out << "Scenario" << mode;
    std::string funcName = out.str();

    // find the method in map
    auto i = funcMap.find(funcName.c_str());
    if(i == funcMap.end())
        throw omnetpp::cRuntimeError("Method %s not found!", funcName.c_str());

    // and then call it
    pfunc f = i->second;
    (this->*f)();

    if(LOG_ACTIVE(DEBUG_LOG_VAL))
    {
        LOG_DEBUG << "\n>>> AddMobileNode is done adding nodes. Here is a summary: \n" << std::flush;
        printLoadedStatistics();
    }
}


void AddMobileNode::printLoadedStatistics()
{
    //###################################
    // Get the list of all possible route
    //###################################

    std::list<std::string> loadedRouteList = TraCI->routeGetIDList();
    LOG_DEBUG << boost::format("  %1% routes are loaded: \n      ") % loadedRouteList.size();
    for(std::string route : loadedRouteList)
        LOG_DEBUG << boost::format("%1%, ") % route;

    LOG_DEBUG << "\n";

    //##################################
    // Get the list of all vehicle types
    //##################################

    std::list<std::string> loadedVehTypeList = TraCI->vehicleTypeGetIDList();
    LOG_DEBUG << boost::format("  %1% vehicle/bike types are loaded: \n      ") % loadedVehTypeList.size();
    for(std::string type : loadedVehTypeList)
        LOG_DEBUG << boost::format("%1%, ") % type;

    LOG_DEBUG << "\n";

    //#############################
    // Get the list of all vehicles
    //#############################

    std::list<std::string> loadedVehList = TraCI->simulationGetLoadedVehiclesIDList();
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

    LOG_FLUSH;
}


// adding 'numVehicles' vehicles with type 'vehiclesType'
// according to deterministic distribution every 'interval'
void AddMobileNode::Scenario1()
{
    int numVehicles = par("numVehicles").longValue();
    std::string vehiclesType = par("vehiclesType").stringValue();
    int interval = par("interval").longValue();

    int depart = 0;

    for(int i=1; i<=numVehicles; i++)
    {
        char vehicleName[90];
        sprintf(vehicleName, "veh%d", i);
        depart = depart + interval;

        TraCI->vehicleAdd(vehicleName, vehiclesType, "route1", depart, 0 /*pos*/, 0 /*speed*/, 0 /*lane*/);
    }
}


// adding 'numVehicles' vehicles with type 'vehiclesType'
// according to Poisson distribution with rate lambda
void AddMobileNode::Scenario2()
{
    int numVehicles = par("numVehicles").longValue();
    std::string vehiclesType = par("vehiclesType").stringValue();
    double lambda = par("lambda").longValue();

    // change from 'veh/h' to 'veh/s'
    lambda = lambda / 3600;

    // 1 vehicle per 'interval' milliseconds
    double interval = (1 / lambda) * 1000;

    int depart = 0;

    for(int i=0; i<numVehicles; i++)
    {
        char vehicleName[90];
        sprintf(vehicleName, "veh%d", i+1);
        depart = depart + interval;

        TraCI->vehicleAdd(vehicleName, vehiclesType, "route1", depart, 0 /*pos*/, 0 /*speed*/, 0 /*lane*/);
    }

    // todo: use this instead! (it throws error)
    //        // mersenne twister engine (seed is fixed to make tests reproducible)
    //        std::mt19937 generator(43);
    //
    //        // Poisson distribution with rate lambda veh/h
    //        std::poisson_distribution<int> distribution(lambda/3600.);
    //
    //        // how many vehicles are inserted in each seconds
    //        int vehInsert;
    //
    //        // how many vehicles are inserted until now
    //        int vehCount = 1;
    //
    //        // on each second
    //        for(int depart=0; depart < terminate; depart++)
    //        {
    //            vehInsert = distribution(generator);
    //
    //            for(int j=1; j<=vehInsert; ++j)
    //            {
    //                char vehicleName[90];
    //                sprintf(vehicleName, "veh%d", vehCount);
    //                TraCI->vehicleAdd(vehicleName, vehiclesType, "route1", 1000*depart, 0 /*pos*/, 0 /*speed*/, 0 /*lane*/);
    //
    //                vehCount++;
    //
    //                if(vehCount > 3)
    //                    return;
    //            }
    //        }
}


// CRL distribution
void AddMobileNode::Scenario4()
{


}


// todo: change this scenario (background traffic)
void AddMobileNode::Scenario5()
{
    int depart = 0;

    for(int i=1; i<=10; i++)
    {
        char vehicleName[90];
        sprintf(vehicleName, "veh%d", i);
        depart = depart + 1000;

        TraCI->vehicleAdd(vehicleName, "TypeCACC1", "route1", depart, 0 /*pos*/, 0 /*speed*/, 0 /*lane*/);
    }

    for(int i=11; i<=100; i++)
    {
        char vehicleName[90];
        sprintf(vehicleName, "veh%d", i);
        depart = depart + 10000;

        TraCI->vehicleAdd(vehicleName, "veh1", "route1", depart, 0 /*pos*/, 0 /*speed*/, 0 /*lane*/);
    }
}


void AddMobileNode::Scenario6()
{
    int numVehicles = par("numVehicles").longValue();
    double lambda = par("lambda").longValue();
    int plnSize = par("plnSize").longValue();
    double plnSpace = par("plnSpace").doubleValue();

    // change from 'veh/h' to 'veh/s'
    lambda = lambda / 3600;

    // 1 vehicle per 'interval' milliseconds
    //double interval = (1 / lambda) * 1000;

    double interval = 5000;

    int depart = 0;

    TraCI->laneSetMaxSpeed("1to2_0", 400.);
    TraCI->vehicleTypeSetMaxSpeed("TypeCACC1", 400.);
    TraCI->vehicleTypeSetVint("TypeCACC1", 400.);
    TraCI->vehicleTypeSetComfAccel("TypeCACC1", 400.);

    for(int i=0; i<numVehicles; i++)
    {
        char vehicleName[90];
        sprintf(vehicleName, "veh%d", i+1);
        depart = depart + interval;

        TraCI->vehicleAdd(vehicleName, "TypeCACC1", "route1", depart, 0 /*pos*/, 0 /*speed*/, 0 /*lane*/);

        if(i == 0)
        {
            TraCI->vehicleSetSpeed(vehicleName, 20.);
        }
        else
        {
            TraCI->vehicleSetSpeed(vehicleName, 400.);
            TraCI->vehicleSetMaxAccel(vehicleName, 400.);
        }

        if(i % plnSize == 0)
        {
            TraCI->vehicleSetTimeGap(vehicleName, plnSpace);
        }
    }
}


// incident detection
void AddMobileNode::Scenario7()
{
    int numVehicles = par("numVehicles").longValue();

    int depart = 0;

    for(int i=1; i<=numVehicles; i++)
    {
        char vehicleName[90];
        sprintf(vehicleName, "veh%d", i);
        depart = depart + 9000;

        uint8_t lane = intrand(3);  // random number in [0,3)

        TraCI->vehicleAdd(vehicleName, "TypeManual", "route1", depart, 0, 0, lane);
        TraCI->vehicleSetLaneChangeMode(vehicleName, 0b1000010101 /*0b1000100101*/);
    }

    // now we add a vehicle as obstacle
    TraCI->vehicleAdd("obstacle", "TypeObstacle", "route1", 50, 3200, 0, 1);

    // and make it stop on the lane!
    TraCI->vehicleSetSpeed("obstacle", 0.);
    TraCI->vehicleSetLaneChangeMode("obstacle", 0);

    // and change its color to red
    RGB newColor = Color::colorNameToRGB("red");
    TraCI->vehicleSetColor("obstacle", newColor);
}


std::vector<std::string> getEdgeNames(std::string netName) {
    std::vector<std::string> edgeNames;

    rapidxml::file <> xmlFile(netName.c_str());
    rapidxml::xml_document<> doc;
    rapidxml::xml_node<> *node;
    doc.parse<0>(xmlFile.data());
    for(node = doc.first_node()->first_node("edge"); node; node = node->next_sibling("edge"))
        edgeNames.push_back(node->first_attribute()->value());

    return edgeNames;
}

std::vector<std::string> getNodeNames(std::string netName) {
    std::vector<std::string> nodeNames;
    rapidxml::file <> xmlFile(netName.c_str());
    rapidxml::xml_document<> doc;
    rapidxml::xml_node<> *node;
    doc.parse<0>(xmlFile.data());
    for(node = doc.first_node()->first_node("junction"); node; node = node->next_sibling("junction"))
        nodeNames.push_back(node->first_attribute()->value());

    return nodeNames;
}

double curve(double x)  //Input will linearly increase from 0 to 1, from first to last vehicle.
{                       //Output should be between 0 and 1, scaled by some function
    return x;
}

void generateVehicles(std::string dir, Router* r)
{
    std::string netName = dir + "/hello.net.xml";

    std::vector<std::string> edgeNames = getEdgeNames(netName);
    std::vector<std::string> nodeNames = getNodeNames(netName);

    srand(time(NULL));
    std::string vName = dir + "/Vehicles" + std::to_string(r->totalVehicleCount) + ".xml";
    std::ofstream vFile(vName.c_str());
    vFile << "<vehicles>" << std::endl;
    for(int i = 1; i <= r->totalVehicleCount; i++)
    {
        std::string edge = edgeNames[rand() % edgeNames.size()];
        std::string node = nodeNames[rand() % nodeNames.size()];
        //vFile << "   <vehicle id=\"v" << i << "\" type=\"TypeManual\" origin=\"" << edge << "\" destination=\"" << node << "\" depart=\"" << i * r->createTime / r->totalVehicleCount << "\" />" << endl;

        vFile << "   <vehicle id=\"v" << i << "\" type=\"TypeManual\" origin=\"" << edge << "\" destination=\""
                << node << "\" depart=\"" << curve((double)i/r->totalVehicleCount) * r->createTime << "\" />" << std::endl;
    }
    vFile << "</vehicles>" << std::endl;
    vFile.close();
}

void AddMobileNode::Scenario8()
{
    cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("router");
    Router *r = static_cast< Router* >(module);

    std::string vehFile = ("/Vehicles" + std::to_string(r->totalVehicleCount) + ".xml");
    std::string xmlFileName = TraCI->getSUMOFullDir();
    xmlFileName += vehFile;

    if( !boost::filesystem::exists(xmlFileName) )
        generateVehicles(TraCI->getSUMOFullDir(), r);

    rapidxml::file<> xmlFile( xmlFileName.c_str() );          // Convert our file to a rapid-xml readable object
    rapidxml::xml_document<> doc;                             // Build a rapidxml doc
    doc.parse<0>(xmlFile.data());                             // Fill it with data from our file
    rapidxml::xml_node<> *node = doc.first_node("vehicles");  // Parse up to the "nodes" declaration

    std::string id, type, origin, destination;
    double depart;
    for(node = node->first_node("vehicle"); node; node = node->next_sibling()) // For each vehicle
    {
        int readCount = 0;
        for(rapidxml::xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())//For each attribute
        {
            switch(readCount)   //Read that attribute to the right variable
            {
            case 0:
                id = attr->value();
                break;
            case 1:
                type = attr->value();
                break;
            case 2:
                origin = attr->value();
                break;
            case 3:
                destination = attr->value();
                break;
            case 4:
                depart = atof(attr->value());
                break;
            }
            readCount++;
        }

        if(readCount < 5)
            throw omnetpp::cRuntimeError("XML formatted wrong! Not enough elements given for some vehicle.");

        r->net->vehicles[id] = new Vehicle(id, type, origin, destination, depart);

        std::list<std::string> routeList = TraCI->routeGetIDList();   //Get all the routes so far
        if(std::find(routeList.begin(), routeList.end(), origin) == routeList.end())
        {
            std::list<std::string> startRoute;
            startRoute.push_back(origin);   //With just the starting edge
            TraCI->routeAdd(origin /*route ID*/, startRoute);   //And add it to the simulation
        }

        //Send a TraCI add call -- might not need to be *1000.
        TraCI->vehicleAdd(id/*vehID*/, type/*vehType*/, origin/*routeID*/, 1000 * depart, 0/*pos*/, 0/*initial speed*/, 0/*lane*/);

        //Change color of non-rerouting vehicle to green.
        std::string veh = id.substr(1,-1);
        if(find(r->nonReroutingVehicles->begin(), r->nonReroutingVehicles->end(), veh) != r->nonReroutingVehicles->end())
        {
            RGB newColor = Color::colorNameToRGB("green");
            TraCI->vehicleSetColor(id, newColor);
        }
    }
}


void AddMobileNode::Scenario9()
{
    // demand per depart for north inbound
    double pNS = 700. / 3600.;
    double pNW = 700. / 3600.;
    double pNE = 700. / 3600.;

    // demand per depart for south inbound
    double pSN = 700. / 3600.;
    double pSE = 700. / 3600.;
    double pSW = 700. / 3600.;

    // demand per depart for west inbound
    double pWE = 700. / 3600.;
    double pWS = 700. / 3600.;
    double pWN = 700. / 3600.;

    // demand per depart for east inbound
    double pEW = 700. / 3600.;
    double pEN = 700. / 3600.;
    double pES = 700. / 3600.;

    int vehNum = 0;
    char vehicleName[90];

    std::mt19937 generator(43);   // mersenne twister engine (seed is fixed to make tests reproducible)
    std::uniform_real_distribution<> distribution(0,1);  // random numbers have uniform distribution between 0 and 1

    for(int i=0; i<terminate; i++)
    {
        if( distribution(generator) < pNS )
        {
            vehNum++;
            sprintf(vehicleName, "Veh-NS-%d", vehNum);

            TraCI->vehicleAdd(vehicleName, "TypeManual", "route1", i, 0 /*pos*/, 30 /*speed*/, 3 /*lane*/);
        }

        if( distribution(generator) < pNW )
        {
            vehNum++;
            sprintf(vehicleName, "Veh-NW-%d", vehNum);

            TraCI->vehicleAdd(vehicleName, "TypeManual", "route2", i, 0 /*pos*/, 30 /*speed*/, 3 /*lane*/);
        }

        if( distribution(generator) < pNE )
        {
            vehNum++;
            sprintf(vehicleName, "Veh-NE-%d", vehNum);

            TraCI->vehicleAdd(vehicleName, "TypeManual", "route3", i, 0 /*pos*/, 30 /*speed*/, 4 /*lane*/);
        }

        if( distribution(generator) < pSN )
        {
            vehNum++;
            sprintf(vehicleName, "Veh-SN-%d", vehNum);

            TraCI->vehicleAdd(vehicleName, "TypeManual", "route4", i, 0 /*pos*/, 30 /*speed*/, 3 /*lane*/);
        }

        if( distribution(generator) < pSE )
        {
            vehNum++;
            sprintf(vehicleName, "Veh-SE-%d", vehNum);

            TraCI->vehicleAdd(vehicleName, "TypeManual", "route5", i, 0 /*pos*/, 30 /*speed*/, 3 /*lane*/);
        }

        if( distribution(generator) < pSW )
        {
            vehNum++;
            sprintf(vehicleName, "Veh-SW-%d", vehNum);

            TraCI->vehicleAdd(vehicleName, "TypeManual", "route6", i, 0 /*pos*/, 30 /*speed*/, 4 /*lane*/);
        }

        if( distribution(generator) < pWE )
        {
            vehNum++;
            sprintf(vehicleName, "Veh-WE-%d", vehNum);

            TraCI->vehicleAdd(vehicleName, "TypeManual", "route7", i, 0 /*pos*/, 30 /*speed*/, 3 /*lane*/);
        }

        if( distribution(generator) < pWS )
        {
            vehNum++;
            sprintf(vehicleName, "Veh-WS-%d", vehNum);

            TraCI->vehicleAdd(vehicleName, "TypeManual", "route8", i, 0 /*pos*/, 30 /*speed*/, 3 /*lane*/);
        }

        if( distribution(generator) < pWN )
        {
            vehNum++;
            sprintf(vehicleName, "Veh-WN-%d", vehNum);

            TraCI->vehicleAdd(vehicleName, "TypeManual", "route9", i, 0 /*pos*/, 30 /*speed*/, 4 /*lane*/);
        }

        if( distribution(generator) < pEW )
        {
            vehNum++;
            sprintf(vehicleName, "Veh-EW-%d", vehNum);

            TraCI->vehicleAdd(vehicleName, "TypeManual", "route10", i, 0 /*pos*/, 30 /*speed*/, 3 /*lane*/);
        }

        if( distribution(generator) < pEN )
        {
            vehNum++;
            sprintf(vehicleName, "Veh-EN-%d", vehNum);

            TraCI->vehicleAdd(vehicleName, "TypeManual", "route11", i, 0 /*pos*/, 30 /*speed*/, 3 /*lane*/);
        }

        if( distribution(generator) < pES )
        {
            vehNum++;
            sprintf(vehicleName, "Veh-ES-%d", vehNum);

            TraCI->vehicleAdd(vehicleName, "TypeManual", "route12", i, 0 /*pos*/, 30 /*speed*/, 4 /*lane*/);
        }
    }
}


// balanced traffic
void AddMobileNode::Scenario10()
{
    double overlap = par("overlap").doubleValue();

    bool vehMultiClass = par("vehMultiClass").boolValue();
    std::vector<double> vehClassDistribution;
    if(vehMultiClass)
    {
        vehClassDistribution = omnetpp::cStringTokenizer(par("vehClassDist").stringValue(), ",").asDoubleVector();
        if(vehClassDistribution.size() != 2)
            throw omnetpp::cRuntimeError("Two values should be specified for vehicle class distribution!");

        // make sure vehicle class distributions are set correctly
        double totalDist = 0;
        for(double i : vehClassDistribution)
            totalDist += i;
        if(totalDist != 100)
            throw omnetpp::cRuntimeError("vehicle class distributions do not add up to 100 percent!");
    }

    std::vector<double> vehRouteDistribution;
    vehRouteDistribution = omnetpp::cStringTokenizer(par("vehRouteDist").stringValue(), ",").asDoubleVector();
    if(vehRouteDistribution.size() != 3)
        throw omnetpp::cRuntimeError("Three values should be specified for vehicle route distribution!");
    // make sure vehicle route distributions are set correctly
    double totalDist = 0;
    for(double i : vehRouteDistribution)
        totalDist += i;
    if(totalDist != 100)
        throw omnetpp::cRuntimeError("vehicle route distributions do not add up to 100 percent!");

    bool bike = par("bike").boolValue();
    std::vector<double> bikeRouteDistribution;
    if(bike)
    {
        bikeRouteDistribution = omnetpp::cStringTokenizer(par("bikeRouteDist").stringValue(), ",").asDoubleVector();
        if(bikeRouteDistribution.size() != 3)
            throw omnetpp::cRuntimeError("Three values should be specified for bike route distribution!");

        // make sure vehicle bike route distributions are set correctly
        totalDist = 0;
        for(double i : bikeRouteDistribution)
            totalDist += i;
        if(totalDist != 100)
            throw omnetpp::cRuntimeError("bike route distributions do not add up to 100 percent!");
    }

    // mersenne twister engine (seed is fixed to make tests reproducible)
    std::mt19937 generator(43);

    // uniform distribution for vehicle route (through, left, right)
    std::uniform_real_distribution<> vehRouteDist(0,1);

    // uniform distribution for vehicle class (passenger, emergency, bus, truck)
    std::uniform_real_distribution<> vehClassDist(0,1);

    // uniform distribution for bike route (through, left, right)
    std::uniform_real_distribution<> bikeRouteDist(0,1);

    // poisson distribution for vehicle insertion
    std::poisson_distribution<int> distribution1(2./36.);
    std::poisson_distribution<int> distribution2(4./36.);
    std::poisson_distribution<int> distribution3(6./36.);
    std::poisson_distribution<int> distribution4(8./36.);
    std::poisson_distribution<int> distribution5(10./36.);

    // poisson distribution for bike insertion
    std::poisson_distribution<int> distribution6(1./36.);

    const int range = 400;    // traffic demand changes after each range
    std::ostringstream name;  // name is in the form of '100_N_T_1' where 100 is Traffic demand, N is north, T is through, 1 is vehCounter

    int vehCounter = 1;

    int vehInsert = 0;   // number of vehicles that should be inserted (per direction) in each second
    double vehDemand;
    double vehRoute = 0;
    double vehClass = 0;

    int bikeInsert = 0;
    double bikeDemand;
    double bikeRoute = 0;
    double bikeInsertionPos = 600;

    for(int depart = 0; depart < terminate; ++depart)
    {
        vehInsert = -1;
        bikeInsert = -1;

        if(depart >= 0 && depart < range)
        {
            vehInsert = distribution1(generator);
            vehDemand = 200;

            bikeInsert = distribution6(generator);
            bikeDemand = 100;
        }

        if(depart >= (range-overlap) && depart < 2*range)
        {
            vehInsert = distribution2(generator);
            vehDemand = 400;

            bikeInsert = distribution6(generator);
            bikeDemand = 100;
        }

        if(depart >= (2*range-overlap) && depart < 3*range)
        {
            vehInsert = distribution3(generator);
            vehDemand = 600;

            bikeInsert = distribution6(generator);
            bikeDemand = 100;
        }

        if(depart >= (3*range-overlap) && depart < 4*range)
        {
            vehInsert = distribution4(generator);
            vehDemand = 800;

            bikeInsert = distribution6(generator);
            bikeDemand = 100;
        }

        if(depart >= (4*range-overlap) && depart < 5*range)
        {
            vehInsert = distribution5(generator);
            vehDemand = 1000;

            bikeInsert = distribution6(generator);
            bikeDemand = 100;
        }

        if(depart >= (5*range-overlap) && depart < 6*range)
        {
            vehInsert = distribution4(generator);
            vehDemand = 800;

            bikeInsert = distribution6(generator);
            bikeDemand = 100;
        }

        if(depart >= (6*range-overlap) && depart < 7*range)
        {
            vehInsert = distribution3(generator);
            vehDemand = 600;

            bikeInsert = distribution6(generator);
            bikeDemand = 100;
        }

        if(depart >= (7*range-overlap) && depart < 8*range)
        {
            vehInsert = distribution2(generator);
            vehDemand = 400;

            bikeInsert = distribution6(generator);
            bikeDemand = 100;
        }

        if(depart >= (8*range-overlap) && depart < 9*range)
        {
            vehInsert = distribution1(generator);
            vehDemand = 200;

            bikeInsert = distribution6(generator);
            bikeDemand = 100;
        }

        if(vehInsert == -1)
            break;

        for(int count = 0; count < vehInsert; ++count)
        {
            // default vehicle type
            std::string vehType = "passenger";

            if(vehMultiClass)
            {
                vehClass = vehClassDist(generator);

                // passenger vehicle
                if( vehClass >= 0 && vehClass < vehClassDistribution[0]/100. )
                    vehType = "passenger";
                // emergency vehicle
                else if( vehClass >= vehClassDistribution[0]/100. && vehClass <= (vehClassDistribution[0]/100. + vehClassDistribution[1]/100.) )
                    vehType = "emergency";
            }

            vehRoute = vehRouteDist(generator);

            // through vehicle
            if( vehRoute >= 0 && vehRoute < vehRouteDistribution[0]/100. )
            {
                name.str("");
                name << vehDemand << "_N_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement2", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << vehDemand << "_S_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement6", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << vehDemand << "_W_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement8", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << vehDemand << "_E_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement4", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }
            // left vehicle
            else if( vehRoute >= vehRouteDistribution[0]/100. && vehRoute < (vehRouteDistribution[0]/100. + vehRouteDistribution[1]/100.) )
            {
                name.str("");
                name << vehDemand << "_N_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement5", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << vehDemand << "_S_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement1", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << vehDemand << "_W_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement3", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << vehDemand << "_E_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement7", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }
            // right vehicle
            else if( vehRoute >= (vehRouteDistribution[0]/100. + vehRouteDistribution[1]/100.) && vehRoute <= (vehRouteDistribution[0]/100. + vehRouteDistribution[1]/100. + vehRouteDistribution[2]/100.) )
            {
                name.str("");
                name << vehDemand << "_N_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "route1", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << vehDemand << "_S_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "route2", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << vehDemand << "_W_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "route3", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << vehDemand << "_E_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "route4", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }

            vehCounter++;
        }

        // adding bikes
        if(bike)
        {
            for(int count = 0; count < bikeInsert; ++count)
            {
                bikeRoute = bikeRouteDist(generator);

                // through bike
                if( bikeRoute >= 0 && bikeRoute < bikeRouteDistribution[0]/100. )
                {
                    name.str("");
                    name << bikeDemand << "_N_T_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "movement2", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);

                    name.str("");
                    name << bikeDemand << "_S_T_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "movement6", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);

                    name.str("");
                    name << bikeDemand << "_W_T_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "movement8", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);

                    name.str("");
                    name << bikeDemand << "_E_T_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "movement4", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);
                }
                // left bike
                else if( bikeRoute >= bikeRouteDistribution[0]/100. && bikeRoute < (bikeRouteDistribution[0]/100. + bikeRouteDistribution[1]/100.) )
                {
                    name.str("");
                    name << bikeDemand << "_N_L_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "movement5", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);

                    name.str("");
                    name << bikeDemand << "_S_L_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "movement1", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);

                    name.str("");
                    name << bikeDemand << "_W_L_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "movement3", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);

                    name.str("");
                    name << bikeDemand << "_E_L_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "movement7", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);
                }
                // right bike
                else if( bikeRoute >= (bikeRouteDistribution[0]/100. + bikeRouteDistribution[1]/100.) && bikeRoute <= (bikeRouteDistribution[0]/100. + bikeRouteDistribution[1]/100. + bikeRouteDistribution[2]/100.) )
                {
                    name.str("");
                    name << bikeDemand << "_N_R_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "route1", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);

                    name.str("");
                    name << bikeDemand << "_S_R_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "route2", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);

                    name.str("");
                    name << bikeDemand << "_W_R_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "route3", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);

                    name.str("");
                    name << bikeDemand << "_E_R_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "route4", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);
                }

                vehCounter++;
            }
        }
    }
}


// unbalanced traffic
void AddMobileNode::Scenario11()
{
    double overlap = par("overlap").doubleValue();

    bool vehMultiClass = par("vehMultiClass").boolValue();
    std::vector<double> vehClassDistribution;
    if(vehMultiClass)
    {
        vehClassDistribution = omnetpp::cStringTokenizer(par("vehClassDist").stringValue(), ",").asDoubleVector();
        if(vehClassDistribution.size() != 2)
            throw omnetpp::cRuntimeError("Two values should be specified for vehicle class distribution!");

        // make sure vehicle class distributions are set correctly
        double totalDist = 0;
        for(double i : vehClassDistribution)
            totalDist += i;
        if(totalDist != 100)
            throw omnetpp::cRuntimeError("vehicle class distributions do not add up to 100 percent!");
    }

    std::vector<double> vehRouteDistribution;
    vehRouteDistribution = omnetpp::cStringTokenizer(par("vehRouteDist").stringValue(), ",").asDoubleVector();
    if(vehRouteDistribution.size() != 3)
        throw omnetpp::cRuntimeError("Three values should be specified for vehicle route distribution!");
    // make sure vehicle route distributions are set correctly
    double totalDist = 0;
    for(double i : vehRouteDistribution)
        totalDist += i;
    if(totalDist != 100)
        throw omnetpp::cRuntimeError("vehicle route distributions do not add up to 100 percent!");

    bool bike = par("bike").boolValue();
    std::vector<double> bikeRouteDistribution;
    if(bike)
    {
        bikeRouteDistribution = omnetpp::cStringTokenizer(par("bikeRouteDist").stringValue(), ",").asDoubleVector();
        if(bikeRouteDistribution.size() != 3)
            throw omnetpp::cRuntimeError("Three values should be specified for bike route distribution!");

        // make sure vehicle bike route distributions are set correctly
        totalDist = 0;
        for(double i : bikeRouteDistribution)
            totalDist += i;
        if(totalDist != 100)
            throw omnetpp::cRuntimeError("bike route distributions do not add up to 100 percent!");
    }

    // mersenne twister engine (seed is fixed to make tests reproducible)
    std::mt19937 generator(43);

    // uniform distribution for vehicle route (through, left, right)
    std::uniform_real_distribution<> vehRouteDist(0,1);

    // uniform distribution for vehicle class (emergency, passenger, bus, truck)
    std::uniform_real_distribution<> vehClassDist(0,1);

    // uniform distribution for bike route (through, left, right)
    std::uniform_real_distribution<> bikeRouteDist(0,1);

    // vehicles in main street
    std::poisson_distribution<int> distribution1(2./36.);
    std::poisson_distribution<int> distribution2(4./36.);
    std::poisson_distribution<int> distribution3(6./36.);
    std::poisson_distribution<int> distribution4(8./36.);
    std::poisson_distribution<int> distribution5(10./36.);

    // vehicles in side street
    std::poisson_distribution<int> distribution6(2./36.);

    // bikes in main street (no bikes in side street)
    std::poisson_distribution<int> distribution7(1./36.);

    const int range = 400;    // traffic demand for main street changes after each range
    std::ostringstream name;  // name is in the form of '100_N_T_1' where 100 is Traffic demand, N is north, T is through, 1 is vehCounter

    int vehCounter = 1;

    int vehInsertMain = 0;   // number of vehicles that should be inserted from W and E in each second
    int vehInsertSide = 0;   // number of vehicles that should be inserted from N and S in each second
    double vehDemandMain;
    double vehDemandSide;
    double vehRoute = 0;
    double vehClass = 0;

    int bikeInsert = 0;
    double bikeDemand;
    double bikeRoute = 0;
    double bikeInsertionPos = 600;

    for(int depart = 0; depart < terminate; ++depart)
    {
        vehInsertMain = -1;
        vehInsertSide = -1;
        bikeInsert = -1;

        if(depart >= 0 && depart < range)
        {
            vehInsertMain = distribution1(generator);
            vehDemandMain = 200;

            vehInsertSide = distribution6(generator);
            vehDemandSide = 200;

            bikeInsert = distribution7(generator);
            bikeDemand = 100;
        }

        if(depart >= (range-overlap) && depart < 2*range)
        {
            vehInsertMain = distribution2(generator);
            vehDemandMain = 400;

            vehInsertSide = distribution6(generator);
            vehDemandSide = 200;

            bikeInsert = distribution7(generator);
            bikeDemand = 100;
        }

        if(depart >= (2*range-overlap) && depart < 3*range)
        {
            vehInsertMain = distribution3(generator);
            vehDemandMain = 600;

            vehInsertSide = distribution6(generator);
            vehDemandSide = 200;

            bikeInsert = distribution7(generator);
            bikeDemand = 100;
        }

        if(depart >= (3*range-overlap) && depart < 4*range)
        {
            vehInsertMain = distribution4(generator);
            vehDemandMain = 800;

            vehInsertSide = distribution6(generator);
            vehDemandSide = 200;

            bikeInsert = distribution7(generator);
            bikeDemand = 100;
        }

        if(depart >= (4*range-overlap) && depart < 5*range)
        {
            vehInsertMain = distribution5(generator);
            vehDemandMain = 1000;

            vehInsertSide = distribution6(generator);
            vehDemandSide = 200;

            bikeInsert = distribution7(generator);
            bikeDemand = 100;
        }

        if(depart >= (5*range-overlap) && depart < 6*range)
        {
            vehInsertMain = distribution4(generator);
            vehDemandMain = 800;

            vehInsertSide = distribution6(generator);
            vehDemandSide = 200;

            bikeInsert = distribution7(generator);
            bikeDemand = 100;
        }

        if(depart >= (6*range-overlap) && depart < 7*range)
        {
            vehInsertMain = distribution3(generator);
            vehDemandMain = 600;

            vehInsertSide = distribution6(generator);
            vehDemandSide = 200;

            bikeInsert = distribution7(generator);
            bikeDemand = 100;
        }

        if(depart >= (7*range-overlap) && depart < 8*range)
        {
            vehInsertMain = distribution2(generator);
            vehDemandMain = 400;

            vehInsertSide = distribution6(generator);
            vehDemandSide = 200;

            bikeInsert = distribution7(generator);
            bikeDemand = 100;
        }

        if(depart >= (8*range-overlap) && depart < 9*range)
        {
            vehInsertMain = distribution1(generator);
            vehDemandMain = 200;

            vehInsertSide = distribution6(generator);
            vehDemandSide = 200;

            bikeInsert = distribution7(generator);
            bikeDemand = 100;
        }

        if(vehInsertMain == -1)
            break;

        for(int count = 0; count < vehInsertMain; ++count)
        {
            // default vehicle type
            std::string vehType = "passenger";

            if(vehMultiClass)
            {
                vehClass = vehClassDist(generator);

                // passenger vehicle
                if( vehClass >= 0 && vehClass < vehClassDistribution[0]/100. )
                    vehType = "passenger";
                // emergency vehicle
                else if( vehClass >= vehClassDistribution[0]/100. && vehClass <= (vehClassDistribution[0]/100. + vehClassDistribution[1]/100.) )
                    vehType = "emergency";
            }

            vehRoute = vehRouteDist(generator);

            // through
            if( vehRoute >= 0 && vehRoute < vehRouteDistribution[0]/100. )
            {
                name.str("");
                name << vehDemandMain << "_W_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement8", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << vehDemandMain << "_E_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement4", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }
            // left
            else if( vehRoute >= vehRouteDistribution[0]/100. && vehRoute < (vehRouteDistribution[0]/100. + vehRouteDistribution[1]/100.) )
            {
                name.str("");
                name << vehDemandMain << "_W_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement3", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << vehDemandMain << "_E_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement7", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }
            // right
            else if( vehRoute >= (vehRouteDistribution[0]/100. + vehRouteDistribution[1]/100.) && vehRoute <= (vehRouteDistribution[0]/100. + vehRouteDistribution[1]/100. + vehRouteDistribution[2]/100.) )
            {
                name.str("");
                name << vehDemandMain << "_W_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "route3", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << vehDemandMain << "_E_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "route4", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }

            vehCounter++;
        }


        for(int count = 0; count < vehInsertSide; ++count)
        {
            // default vehicle type
            std::string vehType = "passenger";

            if(vehMultiClass)
            {
                vehClass = vehClassDist(generator);

                // passenger vehicle
                if( vehClass >= 0 && vehClass < vehClassDistribution[0]/100. )
                    vehType = "passenger";
                // emergency vehicle
                else if( vehClass >= vehClassDistribution[0]/100. && vehClass <= (vehClassDistribution[0]/100. + vehClassDistribution[1]/100.) )
                    vehType = "emergency";
            }

            vehRoute = vehRouteDist(generator);

            // through
            if( vehRoute >= 0 && vehRoute < vehRouteDistribution[0]/100. )
            {
                name.str("");
                name << vehDemandSide << "_N_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement2", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << vehDemandSide << "_S_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement6", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }
            // left
            else if( vehRoute >= vehRouteDistribution[0]/100. && vehRoute < (vehRouteDistribution[0]/100. + vehRouteDistribution[1]/100.) )
            {
                name.str("");
                name << vehDemandSide << "_N_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement5", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << vehDemandSide << "_S_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "movement1", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }
            // right
            else if( vehRoute >= (vehRouteDistribution[0]/100. + vehRouteDistribution[1]/100.) && vehRoute <= (vehRouteDistribution[0]/100. + vehRouteDistribution[1]/100. + vehRouteDistribution[2]/100.) )
            {
                name.str("");
                name << vehDemandSide << "_N_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "route1", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << vehDemandSide << "_S_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), vehType, "route2", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }

            vehCounter++;
        }

        // adding bikes
        if(bike)
        {
            for(int count = 0; count < bikeInsert; ++count)
            {
                bikeRoute = bikeRouteDist(generator);

                // through bike
                if( bikeRoute >= 0 && bikeRoute < bikeRouteDistribution[0]/100. )
                {
                    name.str("");
                    name << bikeDemand << "_W_T_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "movement8", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);

                    name.str("");
                    name << bikeDemand << "_E_T_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "movement4", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);
                }
                // left bike
                else if( bikeRoute >= bikeRouteDistribution[0]/100. && bikeRoute < (bikeRouteDistribution[0]/100. + bikeRouteDistribution[1]/100.) )
                {
                    name.str("");
                    name << bikeDemand << "_W_L_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "movement3", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);

                    name.str("");
                    name << bikeDemand << "_E_L_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "movement7", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);
                }
                // right bike
                else if( bikeRoute >= (bikeRouteDistribution[0]/100. + bikeRouteDistribution[1]/100.) && bikeRoute <= (bikeRouteDistribution[0]/100. + bikeRouteDistribution[1]/100. + bikeRouteDistribution[2]/100.) )
                {
                    name.str("");
                    name << bikeDemand << "_W_R_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "route3", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);

                    name.str("");
                    name << bikeDemand << "_E_R_" << vehCounter;
                    TraCI->vehicleAdd(name.str(), "bicycle", "route4", 1000*depart, bikeInsertionPos /*pos*/, 0 /*speed*/, -5 /*lane*/);
                }

                vehCounter++;
            }
        }
    }
}


// testing starvation
void AddMobileNode::Scenario12()
{
    bool vehMultiClass = par("vehMultiClass").boolValue();
    std::vector<double> vehClassDistribution;
    if(vehMultiClass)
    {
        vehClassDistribution = omnetpp::cStringTokenizer(par("vehClassDist").stringValue(), ",").asDoubleVector();
        if(vehClassDistribution.size() != 2)
            throw omnetpp::cRuntimeError("Two values should be specified for vehicle class distribution!");

        // make sure vehicle class distributions are set correctly
        double totalDist = 0;
        for(double i : vehClassDistribution)
            totalDist += i;
        if(totalDist != 100)
            throw omnetpp::cRuntimeError("vehicle class distributions do not add up to 100 percent!");
    }

    // add a single bike (north to south)
    TraCI->vehicleAdd("bike1", "bicycle", "movement2", 5000, 600 /*pos*/, 0 /*speed*/, -5 /*lane*/);

    // mersenne twister engine (seed is fixed to make tests reproducible)
    std::mt19937 generator(43);

    // uniform distribution for vehicle class (emergency, passenger, bus, truck)
    std::uniform_real_distribution<> vehClassDist(0,1);

    // vehicles in ES direction
    std::poisson_distribution<int> distribution1(7./36.);

    // vehicles in WE direction
    std::poisson_distribution<int> distribution2(7./36.);

    std::ostringstream name;  // name is in the form of '100_N_T_1' where 100 is Traffic demand, N is north, T is through, 1 is vehCounter

    int vehCounter = 1;
    int vehInsertMain = 0;   // number of vehicles that should be inserted from W and E in each second
    int vehInsertMain2 = 0;  // number of vehicles that should be inserted from N and S in each second
    double vehClass = 0;

    for(int depart = 0; depart < terminate; ++depart)
    {
        vehInsertMain = distribution1(generator);

        // insert vehicles ES
        for(int count = 0; count < vehInsertMain; ++count)
        {
            // vehicle type. In single-class all vehicles are passenger
            std::string vehType = "passenger";
            std::string vehColor = "blue";

            if(vehMultiClass)
            {
                vehClass = vehClassDist(generator);

                // passenger vehicle
                if( vehClass >= 0 && vehClass < vehClassDistribution[0]/100. )
                {
                    vehType = "passenger";
                    vehColor = "blue";
                }
                // emergency vehicle
                else if( vehClass >= vehClassDistribution[0]/100. && vehClass <= (vehClassDistribution[0]/100. + vehClassDistribution[1]/100.) )
                {
                    vehType = "emergency";
                    vehColor = "red";
                }
            }

            name.str("");
            name << "_E_L_" << vehCounter;
            TraCI->vehicleAdd(name.str(), vehType, "movement7", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

            // change vehicle color
            RGB newColor = Color::colorNameToRGB(vehColor);
            TraCI->vehicleSetColor(name.str(), newColor);

            vehCounter++;
        }

        vehInsertMain2 = distribution2(generator);

        // insert vehicles WE
        for(int count = 0; count < vehInsertMain2; ++count)
        {
            // vehicle type. In single-class all vehicles are passenger
            std::string vehType = "passenger";
            std::string vehColor = "blue";

            if(vehMultiClass)
            {
                vehClass = vehClassDist(generator);

                // passenger vehicle
                if( vehClass >= 0 && vehClass < vehClassDistribution[0]/100. )
                {
                    vehType = "passenger";
                    vehColor = "blue";
                }
                // emergency vehicle
                else if( vehClass >= vehClassDistribution[0]/100. && vehClass <= (vehClassDistribution[0]/100. + vehClassDistribution[1]/100.) )
                {
                    vehType = "emergency";
                    vehColor = "red";
                }
            }

            name.str("");
            name << "_W_T_" << vehCounter;
            TraCI->vehicleAdd(name.str(), vehType, "movement8", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

            // change vehicle color
            RGB newColor = Color::colorNameToRGB(vehColor);
            TraCI->vehicleSetColor(name.str(), newColor);

            vehCounter++;
        }
    }
}


void AddMobileNode::addFlow()
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


std::string AddMobileNode::getFullPathToSumoRou(std::string sumoConfigFullPath)
{

    return "";

}

}

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

#include <algorithm>
#include <random>

#include "addNode/AddMobileNode.h"
#include "router/Router.h"
#include "logging/vlog.h"

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
        submode = par("submode").longValue();

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
}


void AddMobileNode::beginLoading()
{
    LOG_DEBUG << "\n>>> AddMobileNode is adding nodes to the simulation ... \n" << std::flush;

    // create a map of functions
    typedef void (AddMobileNode::*pfunc)(void);
    std::map<std::string, pfunc> funcMap;
    funcMap["Scenario1"] = &AddMobileNode::Scenario1;
    funcMap["Scenario2"] = &AddMobileNode::Scenario2;
    funcMap["Scenario3"] = &AddMobileNode::Scenario3;
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
}


// adding 'numVehicles' vehicles with type 'vehiclesType'
// according to deterministic distribution every 'interval'
void AddMobileNode::Scenario1()
{

}


// adding 'numVehicles' vehicles with type 'vehiclesType'
// according to Poisson distribution with rate lambda
void AddMobileNode::Scenario2()
{


}


// hardware in the loop scenario
void AddMobileNode::Scenario3()
{


}


// CRL distribution
void AddMobileNode::Scenario4()
{


}


// todo: change this scenario (background traffic)
void AddMobileNode::Scenario5()
{

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
    std::string xmlFileName = TraCI->getDir_SUMOConfig();
    xmlFileName += vehFile;

    if( !boost::filesystem::exists(xmlFileName) )
        generateVehicles(TraCI->getDir_SUMOConfig(), r);

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

        auto routeList = TraCI->routeGetIDList();   //Get all the routes so far
        if(std::find(routeList.begin(), routeList.end(), origin) == routeList.end())
        {
            std::vector<std::string> startRoute = {origin}; //With just the starting edge
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

}


// balanced traffic
void AddMobileNode::Scenario10()
{

}


// unbalanced traffic
void AddMobileNode::Scenario11()
{

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

}

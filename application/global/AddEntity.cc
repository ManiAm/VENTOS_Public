/****************************************************************************/
/// @file    AddEntity.cc
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  depart author name
/// @date    August 2013
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

#include "AddEntity.h"
#include "Router.h"
#include <algorithm>

namespace VENTOS {

Define_Module(VENTOS::AddEntity);

AddEntity::~AddEntity()
{

}


void AddEntity::initialize(int stage)
{
    if(stage ==0)
    {
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);
        terminate = module->par("terminate").doubleValue();

        on = par("on").boolValue();
        mode = par("mode").longValue();
        totalVehicles = par("totalVehicles").longValue();
        vehiclesType = par("vehiclesType").stringValue();
        lambda = par("lambda").longValue();
        plnSize = par("plnSize").longValue();
        plnSpace = par("plnSpace").doubleValue();
        overlap = par("overlap").doubleValue();
        routeDist = par("routeDist").stringValue();

        routeDistribution = cStringTokenizer(routeDist.c_str(), ",").asDoubleVector();
        if(routeDistribution.size() != 3)
            error("Three values should be specified for route distribution!");

        // make sure route distributions are set correctly
        double totalDist = 0;
        for(double i : routeDistribution)
            totalDist += i;
        if(totalDist != 100)
            error("route distributions do not add up to 100 percent!");

        Signal_executeFirstTS = registerSignal("executeFirstTS");
        simulation.getSystemModule()->subscribe("executeFirstTS", this);

        boost::filesystem::path VENTOS_FullPath = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        boost::filesystem::path SUMO_Path = simulation.getSystemModule()->par("SUMODirectory").stringValue();
        SUMO_FullPath = VENTOS_FullPath / SUMO_Path;
        if( !boost::filesystem::exists( SUMO_FullPath ) )
            error("SUMO directory is not valid! Check it again.");
    }
}


void AddEntity::finish()
{

}


void AddEntity::handleMessage(cMessage *msg)
{

}


void AddEntity::receiveSignal(cComponent *source, simsignal_t signalID, long i)
{
    Enter_Method_Silent();

    if(signalID == Signal_executeFirstTS)
    {
        AddEntity::Add();
    }
}


void AddEntity::Add()
{
    // if dynamic adding is off, return
    if (!on)
        return;

    if(mode == 1)
    {
        Scenario1();
    }
    else if(mode == 4)
    {
        Scenario4();
    }
    else if(mode == 5)
    {
        Scenario5();
    }
    else if(mode == 6)
    {
        Scenario6();
    }
    else if(mode == 7)
    {
        Scenario7();
    }
    else if(mode == 8)
    {
        Scenario8();
    }
    else if(mode == 9)
    {
        Scenario9();
    }
    else if(mode == 10)
    {
        Scenario10();
    }
    else if(mode == 11)
    {
        Scenario11();
    }
    else
    {
        error("not a valid mode!");
    }
}


// adding 'totalVehicles' vehicles with type 'vehiclesType'
void AddEntity::Scenario1()
{
    int depart = 0;

    for(int i=1; i<=totalVehicles; i++)
    {
        char vehicleName[90];
        sprintf(vehicleName, "veh_%s_%d", vehiclesType.c_str(), i);
        depart = depart + 10000;

        TraCI->vehicleAdd(vehicleName, vehiclesType, "route1", depart, 0 /*pos*/, 0 /*speed*/, 0 /*lane*/);
    }
}


// todo: merge into scenario1
void AddEntity::Scenario4()
{
    // change from 'veh/h' to 'veh/s'
    lambda = lambda / 3600;

    // 1 vehicle per 'interval' milliseconds
    double interval = (1 / lambda) * 1000;

    int depart = 0;

    for(int i=0; i<totalVehicles; i++)
    {
        char vehicleName[90];
        sprintf(vehicleName, "CACC%d", i+1);
        depart = depart + interval;

        TraCI->vehicleAdd(vehicleName, "TypeCACC1", "route1", depart, 0 /*pos*/, 0 /*speed*/, 0 /*lane*/);
    }
}


// todo: remove this scenario (background traffic)
void AddEntity::Scenario5()
{
    int depart = 0;

    for(int i=1; i<=10; i++)
    {
        char vehicleName[90];
        sprintf(vehicleName, "CACC%d", i);
        depart = depart + 1000;

        TraCI->vehicleAdd(vehicleName, "TypeCACC1", "route1", depart, 0 /*pos*/, 0 /*speed*/, 0 /*lane*/);
    }

    for(int i=11; i<=100; i++)
    {
        char vehicleName[90];
        sprintf(vehicleName, "CACC%d", i);
        depart = depart + 10000;

        TraCI->vehicleAdd(vehicleName, "TypeCACC1", "route1", depart, 0 /*pos*/, 0 /*speed*/, 0 /*lane*/);
    }
}


void AddEntity::Scenario6()
{
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

    for(int i=0; i<totalVehicles; i++)
    {
        char vehicleName[90];
        sprintf(vehicleName, "CACC%d", i+1);
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
void AddEntity::Scenario7()
{
    int depart = 0;

    for(int i=1; i<=totalVehicles; i++)
    {
        char vehicleName[90];
        sprintf(vehicleName, "Veh%d", i);
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
    TraCIColor newColor = TraCIColor::fromTkColor("red");
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
    std::string vName = dir + "/Vehicles" + SSTR(r->totalVehicleCount) + ".xml";
    std::ofstream vFile(vName.c_str());
    vFile << "<vehicles>" << endl;
    for(int i = 1; i <= r->totalVehicleCount; i++)
    {
        std::string edge = edgeNames[rand() % edgeNames.size()];
        std::string node = nodeNames[rand() % nodeNames.size()];
        //vFile << "   <vehicle id=\"v" << i << "\" type=\"TypeManual\" origin=\"" << edge << "\" destination=\"" << node << "\" depart=\"" << i * r->createTime / r->totalVehicleCount << "\" />" << endl;

        vFile << "   <vehicle id=\"v" << i << "\" type=\"TypeManual\" origin=\"" << edge << "\" destination=\""
                << node << "\" depart=\"" << curve((double)i/r->totalVehicleCount) * r->createTime << "\" />" << endl;
    }
    vFile << "</vehicles>" << endl;
    vFile.close();
}

void AddEntity::Scenario8()
{
    cModule *module = simulation.getSystemModule()->getSubmodule("router");
    Router *r = static_cast< Router* >(module);

    std::string vehFile = ("/Vehicles" + SSTR(r->totalVehicleCount) + ".xml");
    std::string xmlFileName = SUMO_FullPath.string();
    xmlFileName += vehFile;

    if( !boost::filesystem::exists(xmlFileName) )
        generateVehicles(SUMO_FullPath.string(), r);

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
            error("XML formatted wrong! Not enough elements given for some vehicle.");

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
            TraCIColor newColor = TraCIColor::fromTkColor("green");
            TraCI->vehicleSetColor(id, newColor);
        }
    }
}


void AddEntity::Scenario9()
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
void AddEntity::Scenario10()
{
    // mersenne twister engine (seed is fixed to make tests reproducible)
    std::mt19937 generator(43);

    // uniform distribution for vehicle route (through, left, right)
    std::uniform_real_distribution<> distribution(0,1);

    // poisson distribution for vehicle insertion
    std::poisson_distribution<int> distribution1(2./36.);
    std::poisson_distribution<int> distribution2(4./36.);
    std::poisson_distribution<int> distribution3(6./36.);
    std::poisson_distribution<int> distribution4(8./36.);
    std::poisson_distribution<int> distribution5(10./36.);

    const int range = 400;     // traffic demand changes after each range
    std::ostringstream name;  // name is in the form of 'veh_100_N_T_1' where 100 is Traffic demand, N is north, T is through, 1 is vehCounter

    int vehCounter = 1;
    int vehInsert = 0;   // number of vehicles that should be inserted (per direction) in each second
    double vehRoute = 0;
    double demand;

    for(int depart = 0; depart < terminate; ++depart)
    {
        vehInsert = -1;

        if(depart >= 0 && depart < range)
        {
            vehInsert = distribution1(generator);
            demand = 200;
        }

        if(depart >= (range-overlap) && depart < 2*range)
        {
            vehInsert = distribution2(generator);
            demand = 400;
        }

        if(depart >= (2*range-overlap) && depart < 3*range)
        {
            vehInsert = distribution3(generator);
            demand = 600;
        }

        if(depart >= (3*range-overlap) && depart < 4*range)
        {
            vehInsert = distribution4(generator);
            demand = 800;
        }

        if(depart >= (4*range-overlap) && depart < 5*range)
        {
            vehInsert = distribution5(generator);
            demand = 1000;
        }

        if(depart >= (5*range-overlap) && depart < 6*range)
        {
            vehInsert = distribution4(generator);
            demand = 800;
        }

        if(depart >= (6*range-overlap) && depart < 7*range)
        {
            vehInsert = distribution3(generator);
            demand = 600;
        }

        if(depart >= (7*range-overlap) && depart < 8*range)
        {
            vehInsert = distribution2(generator);
            demand = 400;
        }

        if(depart >= (8*range-overlap) && depart < 9*range)
        {
            vehInsert = distribution1(generator);
            demand = 200;
        }

        if(vehInsert == -1)
            break;

        for(int count = 0; count < vehInsert; ++count)
        {
            vehRoute = distribution(generator);

            // through
            if( vehRoute >= 0 && vehRoute < routeDistribution[0]/100. )
            {
                name.str("");
                name << "veh_" << demand << "_N_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement2", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << "veh_" << demand << "_S_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement6", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << "veh_" << demand << "_W_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement8", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << "veh_" << demand << "_E_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement4", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }
            // left
            else if( vehRoute >= routeDistribution[0]/100. && vehRoute < (routeDistribution[0]/100. + routeDistribution[1]/100.) )
            {
                name.str("");
                name << "veh_" << demand << "_N_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement5", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << "veh_" << demand << "_S_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement1", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << "veh_" << demand << "_W_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement3", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << "veh_" << demand << "_E_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement7", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }
            // right
            else if( vehRoute >= (routeDistribution[0]/100. + routeDistribution[1]/100.) && vehRoute < (routeDistribution[0]/100. + routeDistribution[1]/100. + routeDistribution[2]/100.) )
            {
                name.str("");
                name << "veh_" << demand << "_N_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "route1", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << "veh_" << demand << "_S_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "route2", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << "veh_" << demand << "_W_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "route3", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << "veh_" << demand << "_E_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "route4", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }

            vehCounter++;
        }
    }

    //    TraCI->vehicleAdd("Bike1", "TypeBicycle", "route1", 0, 0 /*pos*/, 0 /*speed*/, 2 /*lane*/);
    //    TraCI->vehicleAdd("Bike2", "TypeBicycle", "route4", 0, 0 /*pos*/, 0 /*speed*/, 2 /*lane*/);
    //    TraCI->vehicleAdd("Bike3", "TypeBicycle", "route7", 0, 0 /*pos*/, 0 /*speed*/, 2 /*lane*/);
    //    TraCI->vehicleAdd("Bike4", "TypeBicycle", "route10", 0, 0 /*pos*/, 0 /*speed*/, 2 /*lane*/);
}


// unbalanced traffic
void AddEntity::Scenario11()
{
    // mersenne twister engine (seed is fixed to make tests reproducible)
    std::mt19937 generator(43);

    // uniform distribution for vehicle route (through, left, right)
    std::uniform_real_distribution<> distribution(0,1);

    // for main street
    std::poisson_distribution<int> distribution1(2./36.);
    std::poisson_distribution<int> distribution2(4./36.);
    std::poisson_distribution<int> distribution3(6./36.);
    std::poisson_distribution<int> distribution4(8./36.);
    std::poisson_distribution<int> distribution5(10./36.);

    // for side street
    std::poisson_distribution<int> distribution6(2./36.);

    const int range = 400;     // traffic demand for main street changes after each range
    std::ostringstream name;  // name is in the form of 'veh_100_N_T_1' where 100 is Traffic demand, N is north, T is through, 1 is vehCounter

    int vehCounter = 1;
    int vehInsertMain = 0;   // number of vehicles that should be inserted from W and E in each second
    int vehInsertSide = 0;   // number of vehicles that should be inserted from N and S in each second
    double vehRoute = 0;
    double demand;

    for(int depart = 0; depart < terminate; ++depart)
    {
        vehInsertMain = -1;

        if(depart >= 0 && depart < range)
        {
            vehInsertMain = distribution1(generator);
            vehInsertSide = distribution6(generator);
            demand = 200;
        }

        if(depart >= (range-overlap) && depart < 2*range)
        {
            vehInsertMain = distribution2(generator);
            vehInsertSide = distribution6(generator);
            demand = 400;
        }

        if(depart >= (2*range-overlap) && depart < 3*range)
        {
            vehInsertMain = distribution3(generator);
            vehInsertSide = distribution6(generator);
            demand = 600;
        }

        if(depart >= (3*range-overlap) && depart < 4*range)
        {
            vehInsertMain = distribution4(generator);
            vehInsertSide = distribution6(generator);
            demand = 800;
        }

        if(depart >= (4*range-overlap) && depart < 5*range)
        {
            vehInsertMain = distribution5(generator);
            vehInsertSide = distribution6(generator);
            demand = 1000;
        }

        if(depart >= (5*range-overlap) && depart < 6*range)
        {
            vehInsertMain = distribution4(generator);
            vehInsertSide = distribution6(generator);
            demand = 800;
        }

        if(depart >= (6*range-overlap) && depart < 7*range)
        {
            vehInsertMain = distribution3(generator);
            vehInsertSide = distribution6(generator);
            demand = 600;
        }

        if(depart >= (7*range-overlap) && depart < 8*range)
        {
            vehInsertMain = distribution2(generator);
            vehInsertSide = distribution6(generator);
            demand = 400;
        }

        if(depart >= (8*range-overlap) && depart < 9*range)
        {
            vehInsertMain = distribution1(generator);
            vehInsertSide = distribution6(generator);
            demand = 200;
        }

        if(vehInsertMain == -1)
            break;

        for(int count = 0; count < vehInsertMain; ++count)
        {
            vehRoute = distribution(generator);

            // through
            if( vehRoute >= 0 && vehRoute < routeDistribution[0]/100. )
            {
                name.str("");
                name << "veh_" << demand << "_W_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement8", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << "veh_" << demand << "_E_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement4", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }
            // left
            else if( vehRoute >= routeDistribution[0]/100. && vehRoute < (routeDistribution[0]/100. + routeDistribution[1]/100.) )
            {
                name.str("");
                name << "veh_" << demand << "_W_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement3", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << "veh_" << demand << "_E_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement7", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }
            // right
            else if( vehRoute >= (routeDistribution[0]/100. + routeDistribution[1]/100.) && vehRoute < (routeDistribution[0]/100. + routeDistribution[1]/100. + routeDistribution[2]/100.) )
            {
                name.str("");
                name << "veh_" << demand << "_W_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "route3", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << "veh_" << demand << "_E_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "route4", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }

            vehCounter++;
        }


        for(int count = 0; count < vehInsertSide; ++count)
        {
            vehRoute = distribution(generator);

            // through
            if( vehRoute >= 0 && vehRoute < routeDistribution[0]/100. )
            {
                name.str("");
                name << "veh_" << demand << "_N_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement2", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << "veh_" << demand << "_S_T_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement6", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }
            // left
            else if( vehRoute >= routeDistribution[0]/100. && vehRoute < (routeDistribution[0]/100. + routeDistribution[1]/100.) )
            {
                name.str("");
                name << "veh_" << demand << "_N_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement5", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << "veh_" << demand << "_S_L_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "movement1", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }
            // right
            else if( vehRoute >= (routeDistribution[0]/100. + routeDistribution[1]/100.) && vehRoute < (routeDistribution[0]/100. + routeDistribution[1]/100. + routeDistribution[2]/100.) )
            {
                name.str("");
                name << "veh_" << demand << "_N_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "route1", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);

                name.str("");
                name << "veh_" << demand << "_S_R_" << vehCounter;
                TraCI->vehicleAdd(name.str(), "passenger", "route2", 1000*depart, 0 /*pos*/, 0 /*speed*/, -5 /*lane*/);
            }

            vehCounter++;
        }
    }
}


}

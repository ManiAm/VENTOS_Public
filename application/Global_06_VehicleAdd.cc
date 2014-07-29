
#include "Global_06_VehicleAdd.h"

namespace VENTOS {

Define_Module(VENTOS::VehicleAdd);

VehicleAdd::~VehicleAdd()
{

}


void VehicleAdd::initialize(int stage)
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

        on = par("on").boolValue();
        mode = par("mode").longValue();
        totalVehicles = par("totalVehicles").longValue();
        lambda = par("lambda").longValue();
    }
}


void VehicleAdd::handleMessage(cMessage *msg)
{

}


void VehicleAdd::Add()
{
    // if dynamic adding is off, return
    if (!on)
        return;

    // incident detection
    if(mode == 1)
    {
        Scenario1();
    }
    // all ACC
    else if(mode == 2)
    {
        Scenario2();
    }
    // all CACC
    else if(mode == 3)
    {
        Scenario3();
    }
    else if(mode == 4)
    {
        Scenario4();
    }
    else if(mode == 5)
    {
        Scenario5();
    }
    else
    {
        error("not a valid mode!");
    }
}


void VehicleAdd::Scenario1()
{
    int depart = 0;

    for(int i=1; i<=totalVehicles; i++)
    {
        char vehicleName[10];
        sprintf(vehicleName, "Veh%d", i);
        depart = depart + 9000;

        uint8_t lane = intrand(3);  // random number in [0,3)

        TraCI->commandAddVehicleN(vehicleName, "TypeManual", "route1", depart, 0, 0, lane);
        TraCI->commandSetLaneChangeMode(vehicleName, 0b1000010101 /*0b1000100101*/);
    }

    // now we add a vehicle as obstacle
    TraCI->commandAddVehicleN("obstacle", "TypeObstacle", "route1", 50, 3200, 0, 1);

    // make it stop on the lane!
    TraCI->commandSetSpeed("obstacle", 0.);
    TraCI->commandSetLaneChangeMode("obstacle", 0);

    // change the color to blue
    TraCIColor newColor = TraCIColor::fromTkColor("red");
    TraCI->getCommandInterface()->setColor("obstacle", newColor);
}


void VehicleAdd::Scenario2()
{
    int depart = 0;

    for(int i=1; i<=totalVehicles; i++)
    {
        char vehicleName[10];
        sprintf(vehicleName, "ACC%d", i);
        depart = depart + 10000;

        TraCI->commandAddVehicleN(vehicleName, "TypeACC", "route1", depart, 0 /*pos*/, 0 /*speed*/, 0 /*lane*/);
    }
}


void VehicleAdd::Scenario3()
{
    int depart = 0;

     for(int i=1; i<=totalVehicles; i++)
     {
         char vehicleName[10];
         sprintf(vehicleName, "CACC%d", i);
         depart = depart + 10000;

         TraCI->commandAddVehicleN(vehicleName, "TypeCACC", "route1", depart, 0 /*pos*/, 0 /*speed*/, 0 /*lane*/);
     }
}


void VehicleAdd::Scenario4()
{
    boost::filesystem::path SUMODirectory = simulation.getSystemModule()->par("SUMODirectory").stringValue();
    boost::filesystem::path VENTOSfullDirectory = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
    string xmlFileName = (VENTOSfullDirectory / SUMODirectory / "/Vehicles.xml").string();

    file<> xmlFile( xmlFileName.c_str() );     // Convert our file to a rapid-xml readable object
    xml_document<> doc;                        // Build a rapidxml doc
    doc.parse<0>(xmlFile.data());              // Fill it with data from our file
    xml_node<> *node = doc.first_node("vehicles"); // Parse up to the "nodes" declaration

    string id, type, origin, destination;
    int depart;
    for(node = node->first_node("vehicle"); node; node = node->next_sibling()) // For each vehicle
    {
        int readCount = 0;
        for(xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())//For each attribute
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
                depart = atoi(attr->value());
                break;
            }
            readCount++;
        }
        if(readCount < 5)
        {
            error("XML formatted wrong! Not enough elements given for some vehicle.");
        }

        list<string> routeList = TraCI->commandGetRouteIds();   //Get all the routes so far
        bool foundRoute = 0;
        for(list<string>::iterator it = routeList.begin(); it != routeList.end(); it++)   //Loop through them
        {
            //cout << "Found route " << *it << endl;
            if(*it == origin)   //If we find the route named after this vehicle's starting edge, do nothing
            {
                foundRoute = 1;
            }
        }
        if(!foundRoute) //Otherwise, build a new route
        {
            //cout << "Made route " << origin << endl;
            list<string> startRoute;
            startRoute.push_back(origin);   //With just the starting edge
            TraCI->commandAddRoute(origin, startRoute);   //And add it to the simulation
        }

        //cout << "Routes" << endl;
        //for(list<string>::iterator it = routeList.begin(); it != routeList.end(); it++)
        //    cout << *it << endl;

        //commandAddVehicleRouter wants string id, string type, string (edge) origin, string (node) destination, double (time) depart, and string routename
        TraCI->commandAddVehicleN(id, type, origin, 1000 * depart, 0 /*pos*/, 0 /*speed*/, 0 /*lane*/);  //Send a TraCI add call -- might not need to be *1000.
    }
}


void VehicleAdd::Scenario5()
{
    // get simulation time step from TraCI module
    cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
    ASSERT(module);
    double timeStep = module->par("updateInterval").doubleValue();

    // change from "veh/h" to "veh/timeStep"
    lambda = lambda / (3600. * timeStep);

     for(int i=1; i<=totalVehicles; i++)
     {
         char vehicleName[10];
         sprintf(vehicleName, "CACC%d", i);
         int depart = poisson(lambda);

         TraCI->commandAddVehicleN(vehicleName, "TypeCACC", "route1", depart, 0 /*pos*/, 0 /*speed*/, 0 /*lane*/);
     }
}


void VehicleAdd::finish()
{


}
}

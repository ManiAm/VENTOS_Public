
// This file is adopted from TraCIScenarioManagerLaunchd.ned in Veins framework

package c3po.application.traci;

simple TraCIStart
{
    parameters:
        @display("i=block/network2");
        @class(VENTOS::TraCI_Start);
        
        bool debug = default(false);           // emit debug messages?
        
        string host = default("localhost");    // sumo-launchd.py server hostname
        string SUMOexe = default("sumo");
        int seed = default(-1);                // seed value to set in launch configuration, if missing (-1: current run number)
        string SUMOconfig = default("");       // launch configuration to send to sumo-launchd.py
        int margin = default(25);              // margin to add to all received vehicle positions 
        
        double connectAt @unit("s") = default(0s);        // when to connect to TraCI server (must be the initial timestep of the server)
        double firstStepAt @unit("s") = default(-1s);     // when to start synchronizing with the TraCI server (-1: immediately after connecting)
        double updateInterval @unit("s") = default(1s);   // time interval of hosts' position updates
        
        double terminate = default(200s) @unit(s);   // maximum simulation time
        bool autoShutdown = default(true);           // Shutdown module as soon as no more vehicles are in the simulation
        bool TraCIclosed = default(false);           // for internal use
        bool equilibrium_vehicle;
        bool logTraCIcommands = default(false);
        
        string vehicleModuleType = default("inet.nodes.wireless.WirelessHostSimplified");    // module type to be used in the simulation for each managed vehicle
        string vehicleModuleName = default("host");                                          // module name to be used in the simulation for each managed vehicle
        string vehicleModuleDisplayString = default("i=misc/node2;is=vs;r=0,,#707070,1");    // module displayString to be used in the simulation for each managed vehicle  
        
        string bikeModuleType = default("inet.nodes.wireless.WirelessHostSimplified");  // module type to be used in the simulation for each managed bicycle       
        string bikeModuleName = default("host");                                        // module name to be used in the simulation for each managed bicycle        
        string bikeModuleDisplayString = default("i=misc/node2;is=vs;r=0,,#707070,1");  // module displayString to be used in the simulation for each managed bicycle
                
        string pedModuleType = default("inet.nodes.wireless.WirelessHostSimplified");   // module type to be used in the simulation for each managed pedestrian       
        string pedModuleName = default("host");                                         // module name to be used in the simulation for each managed pedestrian        
        string pedModuleDisplayString = default("i=misc/node2;is=vs;r=0,,#707070,1");   // module displayString to be used in the simulation for each managed pedestrian
        
        int numVehicles = default(0);
        double penetrationRate = default(1);          //the probability of a vehicle being equipped with Car2X technology
        int vehicleRngIndex = default(0);             // index of the RNG stream to be used, all random numbers concerning the managed vehicles
        bool useRouteDistributions = default(false);
        
        string roiRoads = default("");  // which roads (e.g. "hwy1 hwy2") are considered to consitute the region of interest, if not empty
        string roiRects = default("");  // which rectangles (e.g. "0,0-10,10 20,20-30,30) are considered to consitute the region of interest, if not empty (in SUMO coordinate)
        double roiSquareSizeRSU = default(0);
}

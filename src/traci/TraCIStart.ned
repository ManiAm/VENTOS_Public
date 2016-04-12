
// This file is adopted from TraCIScenarioManagerLaunchd.ned in Veins framework

package VENTOS.src.traci;

simple TraCIStart
{
    parameters:
        @display("i=block/network2");
        @class(VENTOS::TraCI_Start);
        
        bool debug = default(false);           // emit debug messages?
        
        string host = default("localhost");    // sumo-launchd.py server hostname
        string SUMOexe = default("/home/mani/Desktop/VENTOS_SUMO/sumo/bin/sumo-guiD");   // sumoD: comamnd-line interface    sumo-guiD; graphical interface
        string SUMOconfig = default("");       // SUMO configure file relative path
        bool quitOnEnd = default(true);      // quits the GUI when the simulation stops
        bool startAfterLoading = default(true);  // start the simulation after loading
        bool CMDstepLog = default(true);         // show console output of current simulation step
        int seed = default(0);       // seed value to set in launch configuration, if missing (-1: current run number)
        int margin = default(25);    // margin to add to all received vehicle positions 
        
        double connectAt @unit("s") = default(0s);        // when to connect to TraCI server (must be the initial timestep of the server). -1 means no SUMO!
        double firstStepAt @unit("s") = default(-1s);     // when to start synchronizing with the TraCI server (-1: immediately after connecting)
        double updateInterval @unit("s") = default(0.1s);   // time interval of hosts' position updates
        
        double terminate = default(-1s) @unit(s);   // maximum simulation time. -1 means simulate forever!
        bool autoShutdown = default(true);           // Shutdown module as soon as no more vehicles are in the simulation        
        bool TraCIclosed = default(false);           // for internal use
        bool equilibrium_vehicle = default(false);
        
        bool logTraCIcommands = default(false);        
        bool collectVehiclesData = default(false);
        int vehicleDataLevel = default(1);   // 1:record full vehicle data    2:for router scenario   3:for TL
        
        string vehicleModuleType = default("VENTOS.src.vehicle.vehicle");    // module type to be used in the simulation for each managed vehicle
        string vehicleModuleName = default("V");                             // module name to be used in the simulation for each managed vehicle
        string vehicleModuleDisplayString = default("");                     // module displayString to be used in the simulation for each managed vehicle  
        
        string bikeModuleType = default("VENTOS.src.bicycle.bicycle");  // module type to be used in the simulation for each managed bicycle       
        string bikeModuleName = default("B");                           // module name to be used in the simulation for each managed bicycle        
        string bikeModuleDisplayString = default("");                   // module displayString to be used in the simulation for each managed bicycle
                
        string pedModuleType = default("VENTOS.src.pedestrian.pedestrian");   // module type to be used in the simulation for each managed pedestrian       
        string pedModuleName = default("P");                                  // module name to be used in the simulation for each managed pedestrian        
        string pedModuleDisplayString = default("");                          // module displayString to be used in the simulation for each managed pedestrian
        
        double penetrationRate = default(1);          //the probability of a vehicle being equipped with Car2X technology
        bool useRouteDistributions = default(false);
        
        string roiRoads = default("");  // which roads (e.g. "hwy1 hwy2") are considered to consitute the region of interest, if not empty
        string roiRects = default("");  // which rectangles (e.g. "0,0-10,10 20,20-30,30) are considered to consitute the region of interest, if not empty (in SUMO coordinate)
        double roiSquareSizeRSU = default(0);
}

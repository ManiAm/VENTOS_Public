
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
// Second author: Mani Amoozadeh <maniam@ucdavis.edu>

package VENTOS.src.traci;

simple TraCIStart
{
    parameters:
        @display("i=block/network2");
        @class(VENTOS::TraCI_Start);
        
        @signal[initialize_withTraCI]();        
        @signal[executeEachTS]();
        
        bool active = default(false);
        bool runSUMO = default(true);
        bool debug = default(false);  // emit debug messages?
        
        string SUMOapplication = default("sumo-guiD");   // sumoD: command-line interface    sumo-guiD: graphical interface
        string SUMOconfig = default("");     // relative path to the SUMO configure file
        
        // SUMO switches
        bool quitOnEnd = default(false);         // sumo-gui is closed when the simulation stops
        bool startAfterLoading = default(true);  // SUMO simulation starts right after loading
        bool CMDstepLog = default(true);         // simulation log is printed at the end of each step
        int seed = default(0);  // seed for random generation (-1: current run number)
        
        double terminate = default(-1s) @unit(s);   // maximum simulation time. -1 means simulate forever!
        bool autoShutdown = default(true);          // Shutdown module as soon as no more vehicles are in the simulation        
        bool equilibrium_vehicle = default(false);  // arrived vehicles are re-inserted again
        
        string veh_stat_file = default("");
        bool record_sim_stat = default(false);
        string record_sim_list = default("inserted");
        bool record_TraCI_activity = default(false);     // logging all exchanged TraCI commands
        
        int margin = default(25);   // margin to add to all received vehicle positions
        
        double penetrationRate = default(1);   // the probability of a vehicle being DSRC-enabled
        
        string roiRoads = default("");  // which roads (e.g. "hwy1 hwy2") are considered to consitute the region of interest, if not empty
        string roiRects = default("");  // which rectangles (e.g. "0,0-10,10 20,20-30,30) are considered to consitute the region of interest, if not empty (in SUMO coordinate)
        double roiSquareSizeRSU = default(0);
}

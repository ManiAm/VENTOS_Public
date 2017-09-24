
package VENTOS.src.traci;

// Interface for TraCI module
moduleinterface IBaseTraCI
{
        
}

simple TraCIStart like IBaseTraCI
{
    parameters:
        @display("i=block/network2");
        @class(VENTOS::TraCI_Start);
        
        @signal[initializeWithTraCISignal]();        
        @signal[executeEachTimeStepSignal]();
        
        bool active = default(false);   // should SUMO get started?
        int remotePort = default(-1);   // remote TCP server port for SUMO TraCI
        bool forkSUMO = default(true);  // should we fork the SUMO process?
        
        string SUMOapplication = default("sumo-guiD");   // sumoD: command-line interface    sumo-guiD: graphical interface
        string SUMOconfig = default("");     // relative path to the SUMO configure file
        // SUMO options sent from command line
        //    --start: (GUI only) start the simulation after loading
        //    --quit-on-end: (GUI only) quits the GUI when the simulation stops
        //    --no-step-log: disable console output of current simulation step        
        string SUMOcommandLine = default("--start --no-step-log --seed 0");
        
        double terminateTime = default(-1s) @unit(s); // maximum simulation time. -1 means simulate forever!
        bool autoTerminate = default(true);           // terminate simulation as soon as no more vehicles are in the simulation        
        bool equilibrium_vehicle = default(false);    // arrived vehicles are re-inserted
        
        bool record_TraCI_activity = default(false);   // logging all exchanged TraCI commands
        bool debug = default(false);  // emit debug messages?       
        
        int margin = default(25);  // margin to add to all received vehicle positions
        
        string roiRoads = default("");  // which roads (e.g. "hwy1 hwy2") are considered to consitute the region of interest, if not empty
        string roiRects = default("");  // which rectangles (e.g. "0,0-10,10 20,20-30,30) are considered to consitute the region of interest, if not empty (in SUMO coordinate)
        string roiRectsRSU = default("");
}

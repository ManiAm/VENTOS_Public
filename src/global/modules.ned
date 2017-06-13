package VENTOS.src.global;

import VENTOS.src.mobility.BaseMobility;

// Basic utility module for the whole network.
// Provides utility methods and information used by
// the whole network as well as simulation wide
// black board functionality.
simple BaseWorldUtility
{
    parameters:
        @class(BaseWorldUtility);
        @display("i=misc/globe");
        
        double playgroundSizeX @unit(m);  // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m);  // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m);  // z size of the area the nodes are in (in meters)
        
        bool useTorus = default(false);   // use the playground as torus?
        bool use2D = default(false);      // use a 2-dimensional world?
}


// Interface for Statistics module
moduleinterface IBaseStatistics
{
        
}

simple Statistics like IBaseStatistics
{
    parameters:
        @class(VENTOS::Statistics);
        
        bool record_sim_stat = default(false);
        string record_sim_list = default("inserted");
}


simple InductionLoop
{
    parameters:
        @class(VENTOS::LoopDetectors);
        
        bool record_stat = default(false); 
}


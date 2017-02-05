package VENTOS.src.global;

import VENTOS.src.veins.modules.BaseMobility;

simple ConstSpeedMobility extends BaseMobility
{
    parameters:
        @class(VENTOS::ConstSpeedMobility);
        
        bool debug = default(false); // debug switch
        double speed @unit(mps) = default(0mps); // speed of the host (in m/s)
        double updateInterval @unit(s) = default(1s); // time interval to update the hosts position (seconds)
}


simple Statistics
{
    parameters:
        @class(VENTOS::Statistics);
}


simple InductionLoop
{
    parameters:
        @class(VENTOS::LoopDetectors);
        
        bool record_stat = default(false); 
}

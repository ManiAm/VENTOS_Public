package VENTOS.src.global;

import VENTOS.src.MIXIM.modules.BaseMobility;

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
        
        bool reportMAClayerData = default(false);
        bool reportPlnManagerData = default(false);
        bool reportBeaconsData = default(false);
}

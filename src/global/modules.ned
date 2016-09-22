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


import VENTOS.src.veins.Mac1609_4;

simple Mac1609_4_Mod extends Mac1609_4
{
    parameters:
        @class(VENTOS::Mac1609_4_Mod);
        
}


import VENTOS.src.veins.PhyLayer80211p;

module Nic80211p_Mod  // Mod : modified
{
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        output upperControlOut; // control information 
        input upperControlIn; // control information 
        input radioIn; // radioIn gate for sendDirect

    submodules:
        phy80211p: PhyLayer80211p {
            @display("p=69,218;i=block/process_s");
        }

        mac1609_4: Mac1609_4_Mod {
            @display("p=69,82");
        }

    connections:

        radioIn --> phy80211p.radioIn;

        mac1609_4.lowerControlOut --> phy80211p.upperControlIn;
        mac1609_4.lowerLayerOut --> phy80211p.upperLayerIn;
        phy80211p.upperLayerOut --> mac1609_4.lowerLayerIn;
        phy80211p.upperControlOut --> mac1609_4.lowerControlIn;
        
        mac1609_4.upperControlIn <-- upperControlIn;
        mac1609_4.upperLayerIn <-- upperLayerIn;

        mac1609_4.upperLayerOut --> upperLayerOut;
        mac1609_4.upperControlOut --> upperControlOut;
}


simple Plotter
{
    parameters:
        @class(VENTOS::Plotter);
        @display("i=block/network2");
        
        bool active = default(false);
}


simple Statistics
{
    parameters:
        @class(VENTOS::Statistics);
        
        bool reportMAClayerData = default(false);
        bool reportPlnManagerData = default(false);
        bool reportBeaconsData = default(false);
}

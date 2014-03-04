
package c3po.ned;

import org.mixim.modules.phy.PhyLayer80211p;
//import org.car2x.veins.modules.phy.PhyLayer80211p;


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



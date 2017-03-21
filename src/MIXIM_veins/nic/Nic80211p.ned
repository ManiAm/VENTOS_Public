
package VENTOS.src.MIXIM_veins.nic;

import VENTOS.src.MIXIM_veins.nic.mac.IWirelessMac;
import VENTOS.src.MIXIM_veins.nic.phy.IWirelessPhy;

module Nic80211p
{
    parameters:
        string macType;
        string phyType;
        
        bool drawMaxIntfDist = default(true);      
            
    gates:
        input upperLayerIn;     // to upper layers
        output upperLayerOut;   // from upper layers
        output upperControlOut; // control information 
        input upperControlIn;   // control information 
        input radioIn;          // radioIn gate for sendDirect

    submodules:

        mac1609_4: <macType> like IWirelessMac {
            @display("p=69,82");
        }
        
        phy80211p: <phyType> like IWirelessPhy {
            @display("p=69,218;i=block/process_s");
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

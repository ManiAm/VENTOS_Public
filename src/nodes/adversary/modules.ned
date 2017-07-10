
package VENTOS.src.nodes.adversary;

import VENTOS.src.MIXIM_veins.nic.Nic80211p;
import VENTOS.src.mobility.ConstSpeedMobility;

module Adversary
{
    parameters:
        string SUMOID = default("");
        string SUMOType = default("");
        string vehicleClass = default("");
        
        bool DSRCenabled = default(true);
        bool hasOBU = default(false);
        string IPaddress = default("");
        
    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        appl: ApplAdversary {  
            parameters:
                @display("p=60,50");
        }
        
        nic: Nic80211p {
            parameters:
                @display("p=60,166");
        }
        
        mobility: ConstSpeedMobility {
            parameters:
                @display("p=130,172;i=block/cogwheel");
        }
        
    connections:
        nic.upperLayerOut --> appl.lowerLayerIn;
        nic.upperLayerIn <-- appl.lowerLayerOut;
        nic.upperControlOut --> appl.lowerControlIn;
        nic.upperControlIn <-- appl.lowerControlOut;
        
        radioIn --> nic.radioIn;
}


import VENTOS.src.baseAppl.BaseWaveApplLayer;

simple ApplAdversary extends BaseWaveApplLayer
{
    parameters:
        @class(VENTOS::ApplAdversary);
        
        // attackMode = 0 : falsification
        // attackMode = 1 : replay
        // attackMode = 2, jamming        
        int attackMode = default(-1);
        
        double attackStartTime = default(100);
}


package VENTOS.src.adversary;

import VENTOS.src.veins.Nic80211p;
import VENTOS.src.global.ConstSpeedMobility;

module Adversary
{
    parameters:
        bool DSRCenabled = default(true);

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


import VENTOS.src.global.BaseWaveApplLayer;

simple ApplAdversary extends BaseWaveApplLayer
{
    parameters:
        @class(VENTOS::ApplAdversary);
        
        bool active = default(false);
          
        double AttackT = default(100);
        bool falsificationAttack = default(false);  
        bool replayAttack = default(false); 
        bool jammingAttack = default(false);
}

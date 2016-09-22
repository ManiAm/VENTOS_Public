
package VENTOS.src.adversary;

import VENTOS.src.global.Nic80211p_Mod;
import VENTOS.src.global.ConstSpeedMobility;

module Adversary
{
    parameters:

    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        appl: ApplAdversary {  
            parameters:
                @display("p=60,50");
        }

        nic: Nic80211p_Mod {
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


import VENTOS.src.MIXIM.modules.IBaseApplLayer;

simple ApplAdversary like IBaseApplLayer
{
    parameters:
        @class(VENTOS::ApplAdversary);
        
        bool active = default(false); 
        int headerLength = default(256bit) @unit(bit); // header length of the application 
          
        double AttackT = default(100);
        bool falsificationAttack = default(false);  
        bool replayAttack = default(false); 
        bool jammingAttack = default(false);  

    gates:
        input lowerLayerIn;    // from mac layer
        output lowerLayerOut;  // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}

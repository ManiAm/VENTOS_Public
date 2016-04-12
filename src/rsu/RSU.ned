
package VENTOS.src.rsu;

import VENTOS.src.global.*;
import VENTOS.src.MIXIM.modules.*;

module RSU
{
    parameters:
         string mobilityType; //type of the mobility module

    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        appl: ApplRSUManager {
            parameters:
                @display("p=60,50");
        }

        nic: Nic80211p_Mod {
            parameters:
                @display("p=60,166");
        }
        
        mobility: RSUMobility {
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

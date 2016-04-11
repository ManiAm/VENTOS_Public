
package VENTOS.src.pedestrian;

import VENTOS.src.global.*;
import VENTOS.src.MIXIM.modules.*;

module pedestrian
{
    parameters:
        //   string applType; //type of the application layer
        string mobilityType; //type of the mobility module
        
    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        appl: ApplPedManager {    // <applType> like IBaseApplLayer {
            parameters:
                @display("p=60,50");
        }

        nic: Nic80211p_Mod {
            parameters:
                @display("p=60,166");
        }

        mobility: <mobilityType> like IMobility {
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
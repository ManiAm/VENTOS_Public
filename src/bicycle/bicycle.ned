
package VENTOS.src.bicycle;

import VENTOS.src.global.*;
import VENTOS.src.traci.*;
import VENTOS.src.MIXIM.modules.*;

module bicycle
{
    parameters:
        @signal[beaconSignaling]();
        
    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        appl: ApplBikeManager {    // <applType> like IBaseApplLayer {
            parameters:
                @display("p=60,50");
        }

        nic: Nic80211p_Mod {
            parameters:
                @display("p=60,166");
        }

        mobility: TraCIMobilityMod {
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

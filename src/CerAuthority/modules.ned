
package VENTOS.src.CerAuthority;

import VENTOS.src.veins.Nic80211p;
import VENTOS.src.global.ConstSpeedMobility;

module CA
{
    parameters:
        @signal[CRL_pieces]();
        
    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        appl: ApplCA {  
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


import VENTOS.src.MIXIM.modules.IBaseApplLayer;

simple ApplCA like IBaseApplLayer
{
    parameters:
        @class(VENTOS::ApplCA);
        
        bool active = default(false); 
        int headerLength = default(256bit) @unit(bit); // header length of the application    
        
        int InitialWait = default(0);
        int CRLsize = default(1000);
        bool EnableShuffle = default(false);
        bool ErasureCode = default(true);
        
        int NoSegments = default(20);  // number of CRL pieces (only when ErasureCode = false)

        int M = default(20);
        int N = default(30); 
        
        double Pseudonym_lifeTime = default(120s) @unit(s);

    gates:
        input lowerLayerIn;    // from mac layer
        output lowerLayerOut;  // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}


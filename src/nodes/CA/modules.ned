
package VENTOS.src.nodes.CA;

import VENTOS.src.MIXIM_veins.nic.Nic80211p;
import VENTOS.src.mobility.BaseMobility;

module CA
{
    parameters:
        @signal[CRL_pieces]();
        
        string SUMOID = default("");
        string SUMOType = default("");
        string vehicleClass = default("");
        
        bool DSRCenabled = default(true);
        bool hasOBU = default(false);
        string IPaddress = default("");
        
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
        
        mobility: BaseMobility {
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

simple ApplCA extends BaseWaveApplLayer
{
    parameters:
        @class(VENTOS::ApplCA);
        
        bool active = default(false);   
        
        int InitialWait = default(0);
        int CRLsize = default(1000);
        bool EnableShuffle = default(false);
        bool ErasureCode = default(true);
        
        int NoSegments = default(20);  // number of CRL pieces (only when ErasureCode = false)
        
        int M = default(20);
        int N = default(30); 
        
        double Pseudonym_lifeTime = default(120s) @unit(s);
}


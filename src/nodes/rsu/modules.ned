
package VENTOS.src.nodes.rsu;

import VENTOS.src.MIXIM_veins.nic.Nic80211p;
import VENTOS.src.mobility.BaseMobility;

module RSU
{
    parameters:
        string SUMOID = default("");
        string SUMOType = default("");
        string vehicleClass = default("");
        
        bool DSRCenabled = default(true);
        bool hasOBU = default(false);
        string IPaddress = default("");
        
        // TLid that this RSU belongs to (this parameter is set by AddRSU)
        // empty string means this RSU is not associated with any TL
        string myTLid = default("");      

    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        appl: ApplRSUManager {
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

simple ApplRSUBeacon extends BaseWaveApplLayer
{
    parameters:
        @class(VENTOS::ApplRSUBeacon);
}


simple ApplRSUMonitor extends ApplRSUBeacon
{
    parameters:
        @class(VENTOS::ApplRSUMonitor);   
        
        bool record_vehApproach_stat = default(false);  // if RSU should monitor the intersection activity?
}


simple ApplRSUClassify extends ApplRSUMonitor
{
    parameters:
        @class(VENTOS::ApplRSUCLASSIFY);   
        
        bool classifier = default(false);
        
        double trainError_position @unit("m") = default(0m);  // adding error to training data
        double trainError_speed = default(0);
        
        double GPSError_position @unit("m") = default(0m);    // adding error to received beacon
        double GPSError_speed = default(0);    // adding error to received beacon
}


simple ApplRSUAID extends ApplRSUClassify
{
    parameters:
        @class(VENTOS::ApplRSUAID);
        
        bool enableAID = default(false);
        bool printIncidentDetection = default(false);
}


simple ApplRSUCRL extends ApplRSUAID
{
    parameters:
        @class(VENTOS::ApplRSUCRL);
        
        // CRLdistAlg= 0: Not active
        // CRLdistAlg= 1: CRL_RSU_Only
        // CRLdistAlg= 2: CRL_C2C_Epidemic
        // CRLdistAlg= 3: CRL_MPB
        // CRLdistAlg= 4: CRL_ICE
        // CRLdistAlg= 5: CRL_ICEv2

        // CRLdistAlg= 6: CRL_Broadcast           -->  V2V communication: when a node recieves a broadcast beacon, it sends its pieces.
        // CRLdistAlg= 7: CRL_BBroadcast          -->  V2V communication: nodes send their pieces to each other.
        // CRLdistAlg= 8: CRL_C2C_Epidemic_Ideal  -->  No actual usage of IEEE 802.11g frames
        // CRLdistAlg= 9: CRL_NewMPB
        int CRLdistAlg = default(0); 
               
        double CRL_Interval = default(100s) @unit(s);
        
        // tho: maximum time devoted to CRL broadcasting (=0: No time constraints)
        // frame size (bits) = 18624, bitrate (bps) = 2e+06 --> TxTime (s) = 0.009312
        // ex. if tho=0.03s then only 3 CRLs can be sent.
        // ex. if tho=0.1s then only 10 CRLs can be sent.
        double I2V_tho = default(0.1s) @unit(s);
}


simple ApplRSUYourCode extends ApplRSUCRL
{
    parameters:
        @class(VENTOS::ApplRSUYourCode);
        @display("i=block/app2");
}


simple ApplRSUManager extends ApplRSUYourCode
{
    parameters:
        @class(VENTOS::ApplRSUManager);
}

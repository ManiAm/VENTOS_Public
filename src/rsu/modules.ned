
package VENTOS.src.rsu;

import VENTOS.src.veins.Nic80211p;
import VENTOS.src.global.ConstSpeedMobility;

module RSU
{
    parameters:

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

simple ApplRSUBase like IBaseApplLayer
{
    parameters:
        @class(VENTOS::ApplRSUBase);
        
        int headerLength = default(88bit) @unit(bit); // header length of the application     
        bool debug = default(false); // whether to print debug output
        
        bool sendBeacons = default(false); //tell the applayer to periodically send beacons
        volatile double beaconInterval = default(1s) @unit(s); //the intervall between 2 beacon messages
        double maxOffset = default(0.005s) @unit(s); //the maximum delay of messages sent down to avoid synchronous channel access
        int beaconLengthBits = default(512bit) @unit(bit); //the length of a beacon packet
        int beaconPriority = default(3); //the access category of the beacon messages  
        
        string myTLid = default("");    
        string SUMOID = default(""); 

    gates:
        input lowerLayerIn;     // from mac layer
        output lowerLayerOut;   // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}


simple ApplRSUMonitor extends ApplRSUBase
{
    parameters:
        @class(VENTOS::ApplRSUMonitor);   
        
        bool activeDetection = default(false);  // if RSU should monitor the intersection activity?
        bool collectVehApproach = default(false); // only if activeDetection = true
}


simple ApplRSUClassify extends ApplRSUMonitor
{
    parameters:
        @class(VENTOS::ApplRSUCLASSIFY);   
        
        bool classifier = default(false);
        double trainError @unit("m") = default(0m);  // adding error to training data
        double GPSerror @unit("m") = default(0m);   // adding error to received beacon
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
        
        // CRLdistAlg=-1: Not active
        // CRLdistAlg= 0: CRL_RSU_Only
        // CRLdistAlg= 1: CRL_C2C_Epidemic
        // CRLdistAlg= 2: CRL_MPB
        // CRLdistAlg= 3: CRL_ICE
        // CRLdistAlg= 4: CRL_ICEv2

        // CRLdistAlg= 5: CRL_Broadcast           -->  V2V communication: when a node recieves a broadcast beacon, it sends its pieces.
        // CRLdistAlg= 6: CRL_BBroadcast          -->  V2V communication: nodes send their pieces to each other.
        // CRLdistAlg= 7: CRL_C2C_Epidemic_Ideal  --> No actual usage of IEEE 802.11g frames
        // CRLdistAlg= 8: CRL_NewMPB
        int CRLdistAlg = default(-1); 
               
        double CRL_Interval = default(100s) @unit(s);
        double beacon_Interval = default(1s) @unit(s);
        
        // tho: maximum time devoted to CRL broadcasting (=0: No time constraints)
        // frame size (bits) = 18624, bitrate (bps) = 2e+06 --> TxTime (s) = 0.009312
        // ex. if tho=0.03s then only 3 CRLs can be sent.
        // ex. if tho=0.1s then only 10 CRLs can be sent.
        double I2V_tho = default(0.1s) @unit(s);
}


simple ApplRSUManager extends ApplRSUCRL
{
    parameters:
        @class(VENTOS::ApplRSUManager);

}

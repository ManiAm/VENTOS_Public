
package VENTOS.src.nodes.vehicle;

import VENTOS.src.MIXIM_veins.nic.Nic80211p;
import VENTOS.src.mobility.TraCIMobility_Mod;

module vehicle
{
    parameters:
        @signal[beaconSignaling]();
        @signal[system]();
        
        bool record_stat = default(false);
        string record_list = default("vehId | lane | lanePos | speed");
        
        bool record_emission = default(false);
        string emission_list = default("vehId | CO2");
        
        string SUMOID = default("");
        string SUMOType = default("");
        string vehicleClass = default("");
        
        bool DSRCenabled = default(true);
        bool hasOBU = default(false);
        string IPaddress = default("");
        
        bool SUMOvehicleDebug = default(false);  // controling SUMO debug output
        
    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        appl: ApplVManager {
            parameters:
                @display("p=60,50");
        }
        
        nic: Nic80211p {
            parameters:
                @display("p=60,166");
        }
        
        mobility: TraCIMobility_Mod {
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

simple ApplVBeacon extends BaseWaveApplLayer
{
    parameters:
        @class(VENTOS::ApplVBeacon);
        @display("i=block/app2");
        
        double sonarDist = default(900); 
}


simple ApplVDynamicRouting extends ApplVBeacon
{
    parameters:
        @class(VENTOS::ApplVDynamicRouting);
        @display("i=block/app2");
        
        bool requestRoutes = default(false); //tell the applayer to periodically send beacons
        volatile double requestInterval = default(60s) @unit(s); //the intervall between 2 beacon messages
        double maxSystemOffset = default(0.1s) @unit(s); //the maximum delay of messages sent down to avoid synchronous channel access
        int systemMsgLengthBits = default(512bit) @unit(bit); //the length of a beacon packet
        int systemMsgPriority = default(2); //the access category of the beacon messages
        
        double hypertreeUpdateInterval = default(1);        
        int routingMode = default(1);
}


simple ApplVAID extends ApplVDynamicRouting
{
    parameters:
        @class(VENTOS::ApplVAID);       
        
        // Automatic Incident Detection (AID)
        bool AID = default(false);
}


simple ApplVPlatoon extends ApplVAID
{
    parameters:
        @class(VENTOS::ApplVPlatoon);        
        
        bool record_platoon_stat = default(false);
        
        // the following parameters are used internally [Do not change]
        
        // plnMode = 1: no platooning
        // plnMode = 2: platoonin with no management protocol
        // plnMode = 3: platoon management is active
        int plnMode = default(1);
        
        string myPlnID = default("");
        int myPlnDepth = default(-1);
        int plnSize = default(-1);
}


simple ApplVPlatoonMg extends ApplVPlatoon
{
    parameters:
        @class(VENTOS::ApplVPlatoonMg);        
        
        int maxPlatoonSize = default(-1);
        int optPlatoonSize = default(-1);
        
        // inter-platoon time-gap (between platoons)
        double TP = default(3.5s) @unit(s);
        bool adaptiveTG = default(true);
        
        bool entryEnabled = default(false);
        bool mergeEnabled = default(true);
        bool splitEnabled = default(true);
        bool followerLeaveEnabled = default(true);
        bool leaderLeaveEnabled = default(true);
}


simple ApplVYourCode extends ApplVPlatoonMg
{
    parameters:
        @class(VENTOS::ApplVYourCode);
        @display("i=block/app2");
        
        bool sendingData = default(false);
        bool printCtrlData = default(false);
}


simple ApplVManager extends ApplVYourCode
{
    parameters:
        @class(VENTOS::ApplVManager);
        @display("i=block/app2");
                
        bool record_beacon_stat = default(false);
        
        // beacon loss ratio in application layer
        double plr = default(0);  // packet loss ratio (in percent)
        double dropStartTime = default(-1s) @unit(s);
        
        // measurement error
        bool measurementError = default(false);
        double errorGap = default(0.01);
        double errorRelSpeed = default(0.05);
}

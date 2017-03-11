
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
        
        bool signalBeaconing = default(false); // vehicles send beacons using signals rather than wireless broadcast. Any module
                                               // registered to signalBeaconing signal will receive the beacon 
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


simple ApplVPlatoonFormed extends ApplVAID
{
    parameters:
        @class(VENTOS::ApplVPlatoonFormed); 
        
        // plnMode = 1: no platooning
        // plnMode = 2: pre-defined platoon
        // plnMode = 3: platoon management is active
        int plnMode = default(1);
        
        bool record_platoon_stat = default(false);
        
        string preDefinedPlatoonID = default("CACC1"); // only in plnMode = 2
}


simple ApplVPlatoonMg extends ApplVPlatoonFormed
{
    parameters:
        @class(VENTOS::ApplVPlatoonMg); 
        
        int maxPlatoonSize = default(10);
        int optPlatoonSize = default(5);
        
        bool adaptiveTG = default(true);
        double TP = default(3.5s) @unit(s);   // inter-platoon time-gap (between platoons)
        double TG1 = default(0.55s) @unit(s); // intra-platoon time-gap 1 (between followers)
        double TG2 = default(0.6s) @unit(s);  // intra-platoon time-gap 2 (between followers)
        
        int coordinationMode = default(-1);
        
        bool entryEnabled = default(true);
        bool mergeEnabled = default(true);
        bool splitEnabled = default(true);
        bool followerLeaveEnabled = default(true);
        bool leaderLeaveEnabled = default(true);   
}


simple ApplVManager extends ApplVPlatoonMg
{
    parameters:
        @class(VENTOS::ApplVManager);
        @display("i=block/app2");
        
        bool record_beacon_stat = default(false);
        
        // beacon loss ratio in application layer
        double plr = default(0);  // packet loss ratio (percent) in each simulation timeStep
        double droppT = default(30s) @unit(s);
        string droppV = default("");  // plr for which vehicle? empty string means all
        
        // measurement error
        bool measurementError = default(false);
        double errorGap = default(0.01);
        double errorRelSpeed = default(0.05);
}


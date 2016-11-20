
package VENTOS.src.vehicle;

import VENTOS.src.veins.Nic80211p;
import VENTOS.src.traci.TraCIMobilityMod;

module vehicle
{
    parameters:
        @signal[VehicleState]();
        @signal[beaconSignaling]();
        @signal[system]();
        
    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        appl: ApplVManager {    // <applType> like IBaseApplLayer {
            parameters:
                @display("p=60,50");
        }
        
        nic: Nic80211p {
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


import VENTOS.src.MIXIM.modules.IBaseApplLayer;

simple ApplVBase like IBaseApplLayer
{
    parameters:
        @class(VENTOS::ApplVBase);
        
        int headerLength = default(256bit) @unit(bit); //header length of the application        
        bool debug = default(false); //whether to print debug output
        
        string SUMOID = default("");
        string SUMOType = default("");
        string vehicleClass = default("");
        int vehicleClassEnum = default(-1);
        int SUMOControllerType = default(-1);
        int SUMOControllerNumber = default(-1);
        
        bool hasOBU = default(false);
        string IPaddress = default("");

    gates:
        input lowerLayerIn;    // from mac layer
        output lowerLayerOut;  // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}


simple ApplVBeacon extends ApplVBase
{
    parameters:
        @class(VENTOS::ApplVBeacon);
        @display("i=block/app2");
        
        bool DSRCenabled = default(true); // if false, vehicle beaconing is off throughout the simulation and you can not turn it on again!
        double sonarDist = default(900);
        
        // vehicle beacon msg parameters
        bool sendBeacons = default(true); //tell the applayer to periodically send beacons. You can turn beaconing off/on in any point
        volatile double beaconInterval = default(0.1s) @unit(s); //the intervall between 2 beacon messages
        double maxOffset = default(0.005s) @unit(s); //the maximum delay of messages sent down to avoid synchronous channel access
        int beaconLengthBits = default(512bit) @unit(bit); //the length of a beacon packet
        int beaconPriority = default(3); //the access category of the beacon messages
        
        bool signalBeaconing = default(false); // vehicles send beacons using signals rather than wireless broadcast. Any module
                                               // registered to signalBeaconing signal will receive the beacon 
        
        // vehicle data msg parameters
        bool dataOnSch = default(true); //tells the applayer whether to use a service channel for datapackets or the control channel
        int dataLengthBits = default(1024bit) @unit(bit); //the length of a data packet
        int dataPriority = default(2); //the access category for data packets  
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
        
        string preDefinedPlatoonID = default("CACC1"); // only in plnMode = 2
}


simple ApplVPlatoonMg extends ApplVPlatoonFormed
{
    parameters:
        @class(VENTOS::ApplVPlatoonMg); 
        
        int maxPlatoonSize = default(10);
        int optPlatoonSize = default(5);
        
        bool adaptiveTG = default(true);
        double TP = default(3.5s) @unit(s);   // intra-platoon time-gap (between platoons)
        double TG1 = default(0.55s) @unit(s); // inter-platoon time-gap 1 (between followers)
        double TG2 = default(0.6s) @unit(s);  // inter-platoon time-gap 2 (between followers)
        
        int coordinationMode = default(-1);
        
        bool entryEnabled = default(true);
        bool mergeEnabled = default(true);
        bool splitEnabled = default(true);
        bool followerLeaveEnabled = default(true);
        bool leaderLeaveEnabled = default(true);   
}


simple ApplVMsgControl extends ApplVPlatoonMg
{
    parameters:
        @class(VENTOS::ApplVMsgControl);
        @display("i=block/app2");
        
        bool printCtrlData = default(false);
}


simple ApplVManager extends ApplVMsgControl
{
    parameters:
        @class(VENTOS::ApplVManager);
        @display("i=block/app2");
        
        bool SUMOvehicleDebug = default(false);  // controling SUMO debug output for "each vehicle"
        bool degradeToACC = default(true);
        
        // beacon loss ratio in application layer
        double plr = default(0);  // packet loss ratio (percent) in each simulation timeStep
        double droppT = default(30s) @unit(s);
        string droppV = default("");  // plr for which vehicle? empty string means all
        
        // measurement error
        bool measurementError = default(false);
        double errorGap = default(0.01);
        double errorRelSpeed = default(0.05);
}


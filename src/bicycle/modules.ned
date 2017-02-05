
package VENTOS.src.bicycle;

import VENTOS.src.veins.Nic80211p;
import VENTOS.src.traci.TraCIMobilityMod;

module bicycle
{
    parameters:
        @signal[beaconSignaling]();
        
        bool record_stat = default(false);
        string record_list = default("timeStep | id | lane | lanePos | speed");
        
        string SUMOID = default("");
        string SUMOType = default("");
        string vehicleClass = default("");
        
        bool DSRCenabled = default(true);
        bool hasOBU = default(false);
        string IPaddress = default("");
        
    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        appl: ApplBikeManager {    // <applType> like IBaseApplLayer {
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


import VENTOS.src.veins.modules.IBaseApplLayer;

simple ApplBikeBase like IBaseApplLayer
{
    parameters:
        @class(VENTOS::ApplBikeBase);
        
        int headerLength = default(256bit) @unit(bit); //header length of the application        
        bool debug = default(false); //whether to print debug output

    gates:
        input lowerLayerIn;    // from mac layer
        output lowerLayerOut;  // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}


simple ApplBikeBeacon extends ApplBikeBase
{
    parameters:
        @class(VENTOS::ApplBikeBeacon);
        @display("i=block/app2");
                
        bool sendBeacons = default(false); //tell the applayer to periodically send beacons
        volatile double beaconInterval = default(0.1s) @unit(s); //the intervall between 2 beacon messages
        double maxOffset = default(0.005s) @unit(s); //the maximum delay of messages sent down to avoid synchronous channel access
        int beaconLengthBits = default(512bit) @unit(bit); //the length of a beacon packet
        int beaconPriority = default(3); //the access category of the beacon messages
        
        bool signalBeaconing = default(false);
        
        // Bicycle data msg parameters
        bool dataOnSch = default(true); //tells the applayer whether to use a service channel for datapackets or the control channel
        int dataLengthBits = default(1024bit) @unit(bit); //the length of a data packet
        int dataPriority = default(2); //the access category for data packets  
}


simple ApplBikeManager extends ApplBikeBeacon
{
    parameters:
        @class(VENTOS::ApplBikeManager);
        @display("i=block/app2");
}

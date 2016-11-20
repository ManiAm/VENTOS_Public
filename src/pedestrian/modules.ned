
package VENTOS.src.pedestrian;

import VENTOS.src.veins.Nic80211p;
import VENTOS.src.traci.TraCIMobilityMod;

module pedestrian
{
    parameters:
        @signal[beaconSignaling]();
        
    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        appl: ApplPedManager {    // <applType> like IBaseApplLayer {
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

simple ApplPedBase like IBaseApplLayer
{
    parameters:
        @class(VENTOS::ApplPedBase);
        
        int headerLength = default(88bit) @unit(bit); //header length of the application        
        bool debug = default(false); //whether to print debug output
        
        string SUMOID = default("");
        string SUMOType = default("");
        string vehicleClass = default("");
        int vehicleClassEnum = default(-1);
        
        // ped does not need these two varibales but
        // it makes the job simpler for TraCIstart.cc
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


simple ApplPedBeacon extends ApplPedBase
{
    parameters:
        @class(VENTOS::ApplPedBeacon);
        @display("i=block/app2");
        
        bool DSRCenabled = default(true);  // non-changable during simulation
                
        bool sendBeacons = default(false); //tell the applayer to periodically send beacons
        volatile double beaconInterval = default(0.1s) @unit(s); //the intervall between 2 beacon messages
        double maxOffset = default(0.005s) @unit(s); //the maximum delay of messages sent down to avoid synchronous channel access
        int beaconLengthBits = default(512bit) @unit(bit); //the length of a beacon packet
        int beaconPriority = default(3); //the access category of the beacon messages
        
        // Pedestrian data msg parameters
        bool dataOnSch = default(false); //tells the applayer whether to use a service channel for datapackets or the control channel
        int dataLengthBits = default(1024bit) @unit(bit); //the length of a data packet
        int dataPriority = default(2); //the access category for data packets  
}


simple ApplPedManager extends ApplPedBeacon
{
    parameters:
        @class(VENTOS::ApplPedManager);
        @display("i=block/app2");
}



package VENTOS.src.obstacle;

import VENTOS.src.veins.Nic80211p;
import VENTOS.src.traci.TraCIMobilityMod;

module obstacle
{
    parameters:
        @signal[beaconSignaling]();
        
        bool record_stat = default(false);
        string record_list = default("timeStep | id | lane | lanePos | speed");
        
        string SUMOID = default("");
        string SUMOType = default("");
        string vehicleClass = default("");
        
        bool hasOBU = default(false);
        string IPaddress = default("");
        
    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        appl: ApplObstacleManager {    // <applType> like IBaseApplLayer {
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

simple ApplObstacleBase like IBaseApplLayer
{
    parameters:
        @class(VENTOS::ApplObstacleBase);
        
        int headerLength = default(256bit) @unit(bit); //header length of the application        
        bool debug = default(false); //whether to print debug output

    gates:
        input lowerLayerIn;    // from mac layer
        output lowerLayerOut;  // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}


simple ApplObstacleBeacon extends ApplObstacleBase
{
    parameters:
        @class(VENTOS::ApplObstacleBeacon);
        @display("i=block/app2");
        
        bool DSRCenabled = default(true);  // non-changable during simulation
                
        bool sendBeacons = default(false); //tell the applayer to periodically send beacons
        volatile double beaconInterval = default(0.1s) @unit(s); //the intervall between 2 beacon messages
        double maxOffset = default(0.005s) @unit(s); //the maximum delay of messages sent down to avoid synchronous channel access
        int beaconLengthBits = default(512bit) @unit(bit); //the length of a beacon packet
        int beaconPriority = default(3); //the access category of the beacon messages
        
        // data msg parameters
        bool dataOnSch = default(true); //tells the applayer whether to use a service channel for datapackets or the control channel
        int dataLengthBits = default(1024bit) @unit(bit); //the length of a data packet
        int dataPriority = default(2); //the access category for data packets  
}


simple ApplObstacleManager extends ApplObstacleBeacon
{
    parameters:
        @class(VENTOS::ApplObstacleManager);
        @display("i=block/app2");
}

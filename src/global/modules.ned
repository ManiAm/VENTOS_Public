package VENTOS.src.global;

import VENTOS.src.veins.modules.BaseMobility;

simple ConstSpeedMobility extends BaseMobility
{
    parameters:
        @class(VENTOS::ConstSpeedMobility);
        
        bool debug = default(false); // debug switch
        double speed @unit(mps) = default(0mps); // speed of the host (in m/s)
        double updateInterval @unit(s) = default(1s); // time interval to update the hosts position (seconds)
}


simple Statistics
{
    parameters:
        @class(VENTOS::Statistics);
}


simple InductionLoop
{
    parameters:
        @class(VENTOS::LoopDetectors);
        
        bool record_stat = default(false); 
}


import VENTOS.src.veins.modules.IBaseApplLayer;

simple BaseWaveApplLayer like IBaseApplLayer
{
    parameters:
        bool debug = default(false); //whether to print debug output

        int headerLength = default(88bit) @unit(bit); //header length of the application

        bool sendBeacons = default(false); //tell the applayer to periodically send beacons
        int beaconLengthBits = default(256bit) @unit(bit); //the length of a beacon packet
        int beaconPriority = default(3); //the access category of the beacon messages
        double beaconInterval = default(1s) @unit(s); //the intervall between 2 beacon messages

        int dataLengthBits = default(1024bit) @unit(bit); //the length of a data packet
        bool dataOnSCH = default(false); //tells the applayer whether to use a service channel for datapackets or the control channel
        int dataPriority = default(3); //the default access category for data packets

        bool avoidBeaconSynchronization = default(true); //don't start beaconing directly after node was created but delay to avoid artifical synchronization

    gates:
        input lowerLayerIn;    // from mac layer
        output lowerLayerOut;  // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}

package VENTOS.src.baseAppl;

import VENTOS.src.mobility.BaseMobility;

// Base module for all layer modules. Defines the in and out gates
simple BaseLayer
{
    parameters:
        bool notAffectedByHostState = default(false);
        
    gates:
        input upperLayerIn @loose;     // from upper layer
        output upperLayerOut @loose;   // to uppe layer
        input upperControlIn @loose;   // control from upper layer
        output upperControlOut @loose; // control to upper layer
        
        input lowerLayerIn;     // from lower layer
        output lowerLayerOut;   // to lower layer
        input lowerControlIn;   // control from lower layer
        output lowerControlOut; // control to lower layer
}


simple BaseWaveApplLayer extends BaseLayer
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
}

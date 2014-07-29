package c3po.ned;

simple ApplVBeacon extends ApplVBase
{
    parameters:
        @class(VENTOS::ApplVBeacon);
        @display("i=block/app2");
        
        bool VANETenabled = default(true);
        int controlMode = default(0);
        int plnMode = default(0);
        double sonarDist = default(900);
                
        bool sendBeacons = default(false); //tell the applayer to periodically send beacons
        volatile double beaconInterval = default(1s) @unit(s); //the intervall between 2 beacon messages
        double maxOffset = default(0s) @unit(s); //the maximum delay of messages sent down to avoid synchronous channel access
        int beaconLengthBits = default(256bit) @unit(bit); //the length of a beacon packet
        int beaconPriority = default(3); //the access category of the beacon messages
        
        int dataLengthBits = default(1024bit) @unit(bit); //the length of a data packet
        bool dataOnSch = default(true); //tells the applayer whether to use a service channel for datapackets or the control channel
        int dataPriority = default(3); //the access category for data packets  
}

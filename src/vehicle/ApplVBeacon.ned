
package VENTOS.src.vehicle;

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

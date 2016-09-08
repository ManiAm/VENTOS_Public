
package VENTOS.src.vehicle;

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

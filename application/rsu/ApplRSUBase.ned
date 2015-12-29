
package c3po.application.rsu;
import c3po.application.MIXIM.modules.*;

simple ApplRSUBase like IBaseApplLayer
{
    parameters:
        @class(VENTOS::ApplRSUBase);
        
        int headerLength = default(88bit) @unit(bit); // header length of the application     
        bool debug = default(false); // whether to print debug output
        
        bool sendBeacons = default(false); //tell the applayer to periodically send beacons
        volatile double beaconInterval = default(1s) @unit(s); //the intervall between 2 beacon messages
        double maxOffset = default(0s) @unit(s); //the maximum delay of messages sent down to avoid synchronous channel access
        int beaconLengthBits = default(256bit) @unit(bit); //the length of a beacon packet
        int beaconPriority = default(3); //the access category of the beacon messages  
        
        string myTLid;    
        double myCoordX;    // my X coordinate in SUMO
        double myCoordY;    // my Y coordinate in SUMO  

    gates:
        input lowerLayerIn;     // from mac layer
        output lowerLayerOut;   // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}

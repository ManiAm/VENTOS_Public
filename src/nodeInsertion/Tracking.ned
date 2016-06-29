
package VENTOS.src.nodeInsertion;

simple Tracking
{
    parameters:
        @class(VENTOS::Tracking);
        @display("i=block/network2");
        
        // trackingMode = -1: disable
        // trackingMode =  0: zoom in
        // trackingMode =  1: zoom in slowly
        // trackingMode =  2: track a specific vehicle
        // trackingMode =  3: move windows frame when all vehicles exit
        int mode = default(-1);                
                
        double zoom = default(11512);
        double initialWindowsOffset = default(130);
        double trackingInterval = default(1);
        string trackingV = default("veh3");       // in mode 2
        string trackingLane = default("1to2_0");  // in mode 3
        double windowsOffset = default(400);      // in mode 3
}


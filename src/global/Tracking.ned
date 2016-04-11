
package VENTOS.src.global;

simple Tracking
{
    parameters:
        @class(VENTOS::Tracking);
        @display("i=block/network2");
        
        double zoom;
        double initialWindowsOffset;
        double trackingInterval;
        int mode;
        string trackingV;
        string trackingLane;
        double windowsOffset;
}



package c3po.application.global;

simple Tracking
{
    parameters:
        @class(VENTOS::Tracking);
        @display("i=block/network2");
        
        bool on = default(true);
        double zoom;
        double initialWindowsOffset;
        double trackingInterval;
        int mode;
        string trackingV;
        string trackingLane;
        double windowsOffset;
}


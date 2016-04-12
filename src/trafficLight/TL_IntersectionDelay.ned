
package VENTOS.src.trafficLight;

simple IntersectionDelay extends TL_TLStateRecord
{
    parameters:
        @class(VENTOS::IntersectionDelay);
        @display("i=block/network2");  
        
        bool collectVehDelay = default(false);    
        
        double deccelDelayThreshold = default(-1);  // m/s^2: vehicle/bike with decel less than this value is considered delayed
        double vehStoppingDelayThreshold = default(5.0);  // m/s: vehicle with speed less than this value is considered stopped
        double bikeStoppingDelayThreshold = default(0.4); // m/s: bike with speed less than this value is considered stopped 
        int lastValueBuffSize = default(10); // how much further we need to look at speed/accel data to mark the delay start
}

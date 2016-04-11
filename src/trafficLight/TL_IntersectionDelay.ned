
package VENTOS.src.trafficLight;

simple IntersectionDelay extends TL_TLStateRecord
{
    parameters:
        @class(VENTOS::IntersectionDelay);
        @display("i=block/network2");  
        
        bool collectVehDelay;    
        
        double deccelDelayThreshold;
        double vehStoppingDelayThreshold;
        double bikeStoppingDelayThreshold;
        int lastValueBuffSize;
}

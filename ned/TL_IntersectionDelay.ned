
package c3po.ned;

simple IntersectionDelay extends TL_LoopDetectors
{
    parameters:
        @class(VENTOS::IntersectionDelay);
        @display("i=block/network2");  
        
        bool collectVehDelay;    
        
        double deccelDelayThreshold;
        double stoppingDelayThreshold;
        int lastValueBuffSize;
}


package c3po.ned;

simple VehDelay extends TL_LoopDetectors
{
    parameters:
        @class(VENTOS::VehDelay);
        @display("i=block/network2");  
        
        bool collectVehDelay;    
        
        double deccelDelayThreshold;
        double stoppingDelayThreshold;
        int lastValueBuffSize;
}

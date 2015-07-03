
package c3po.ned;

simple VehDelay extends TL_LoopDetectors
{
    parameters:
        @class(VENTOS::VehDelay);
        @display("i=block/network2");  
        
        bool measureVehDelay;    
        double deccelDelayThreshold;
        double stoppingDelayThreshold;
        int lastSpeedBuffSize;
        int lastAccelBuffSize;
}

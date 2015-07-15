
package c3po.ned;

simple TL_AdaptiveQueue extends TL_LowDelay
{
    parameters:
        @class(VENTOS::TrafficLightAdaptiveQueue);
        @display("i=block/network2");  
        
        int maxQueueSize = default(-1);              
}

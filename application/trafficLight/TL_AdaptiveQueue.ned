
package c3po.application.trafficLight;

simple TL_AdaptiveQueue extends TL_Actuated
{
    parameters:
        @class(VENTOS::TrafficLightAdaptiveQueue);
        @display("i=block/network2");  
        
        int maxQueueSize = default(-1);              
}

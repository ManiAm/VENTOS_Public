
package VENTOS.src.trafficLight.TSC;

simple TL_LQF extends TL_Actuated
{
    parameters:
        @class(VENTOS::TrafficLightLQF_NoStarv);
        @display("i=block/network2");  
        
        int maxQueueSize = default(-1);  // max queue has the accurate queue size information            
}


package c3po.application.trafficLight;

simple TL_LongestQueue extends TL_Actuated
{
    parameters:
        @class(VENTOS::TrafficLightLongestQueueNoStarv);
        @display("i=block/network2");  
        
        int maxQueueSize = default(-1);              
}

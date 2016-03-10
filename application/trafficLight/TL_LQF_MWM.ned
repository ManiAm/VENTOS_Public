
package c3po.application.trafficLight;

simple TL_LQF_MWM extends TL_OJF
{
    parameters:
        @class(VENTOS::TrafficLight_LQF_MWM_NoStarv);
        @display("i=block/network2");  
        
}

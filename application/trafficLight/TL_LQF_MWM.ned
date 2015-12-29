
package c3po.application.trafficLight;

simple TL_LQF_MWM extends TL_LowDelay
{
    parameters:
        @class(VENTOS::TrafficLight_LQF_MWM);
        @display("i=block/network2");  
        
}

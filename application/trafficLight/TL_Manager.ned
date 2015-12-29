
package c3po.application.trafficLight;

simple TL_Manager extends TL_Router
{
    parameters:
        @class(VENTOS::TrafficLightManager);
        @display("i=block/network2");  
    
}

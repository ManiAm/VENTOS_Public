
package c3po.application.trafficLight;
import c3po.application.trafficLight.TSC.TL_Router;

simple TL_Manager extends TL_Router
{
    parameters:
        @class(VENTOS::TrafficLightManager);
        @display("i=block/network2");  
    
}

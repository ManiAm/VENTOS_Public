
package VENTOS.src.trafficLight;

import VENTOS.src.trafficLight.TSC.TL_Router;

simple TL_Manager extends TL_Router
{
    parameters:
        @class(VENTOS::TrafficLightManager);
        @display("i=block/network2");  
    
}

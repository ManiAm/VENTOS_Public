
package c3po.ned;

simple TL_Manager extends TL_Router
{
    parameters:
        @class(VENTOS::TrafficLightManager);
        @display("i=block/network2");  
    
}


package c3po.ned;

simple TL_VANET extends TL_AdaptiveQueue
{
    parameters:
        @class(VENTOS::TrafficLightVANET);
        @display("i=block/network2");  
        
}

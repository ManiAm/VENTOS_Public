
package c3po.ned;

simple TL_AdaptiveQueue extends TL_Adaptive
{
    parameters:
        @class(VENTOS::TrafficLightAdaptiveQueue);
        @display("i=block/network2");  
              
}

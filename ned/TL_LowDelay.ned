
package c3po.ned;

simple TL_LowDelay extends TL_AdaptiveQueue
{
    parameters:
        @class(VENTOS::TrafficLightLowDelay);
        @display("i=block/network2");              
}

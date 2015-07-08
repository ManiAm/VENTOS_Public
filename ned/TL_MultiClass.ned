
package c3po.ned;

simple TL_MultiClass extends TL_AdaptiveQueue
{
    parameters:
        @class(VENTOS::TrafficLightMultiClass);
        @display("i=block/network2");  
        
}

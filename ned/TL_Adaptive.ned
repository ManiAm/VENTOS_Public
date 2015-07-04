
package c3po.ned;

simple TL_Adaptive extends TL_Webster
{
    parameters:
        @class(VENTOS::TrafficLightAdaptive);
        @display("i=block/network2");        
        
        bool greenExtension = default(true);
        double passageTime = default(0s) @unit(s);
}

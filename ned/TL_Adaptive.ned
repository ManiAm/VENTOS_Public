
package c3po.ned;

simple TL_Adaptive extends TL_Fixed
{
    parameters:
        @class(VENTOS::TrafficLightAdaptive);
        @display("i=block/network2");  
        
        double minGreenTime = default(0s) @unit(s);
        double maxGreenTime = default(0s) @unit(s);
        double yellowTime = default(0s) @unit(s);
        double redTime = default(0s) @unit(s);
        double passageTime = default(0s) @unit(s);
}

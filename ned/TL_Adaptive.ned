
package c3po.ned;

simple TL_Adaptive extends TL_Fixed
{
    parameters:
        @class(VENTOS::TrafficLightAdaptive);
        @display("i=block/network2");  
        
        double minGreenTime;
        double maxGreenTime;
        double yellowTime;
        double redTime;
        double passageTime;        
}

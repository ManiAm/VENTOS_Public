
package c3po.ned;

simple TrafficLight
{
    parameters:
        @class(VENTOS::TrafficLight);
        @display("i=block/network2");  
        
        int TLLogicMode;
        double HighDensityRecalculateFrequency;
        double LowDensityExtendTime;
        double MaxPhaseDuration;        
}

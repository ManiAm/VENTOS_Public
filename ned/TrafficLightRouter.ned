
package c3po.ned;

simple TrafficLightRouter extends TrafficLightBase
{
    parameters:
        @class(VENTOS::TrafficLightRouter);
        @display("i=block/network2");  
        
        int TLLogicMode;
        double HighDensityRecalculateFrequency;
        double LowDensityExtendTime;
        double MaxPhaseDuration;     
        double MinPhaseDuration;   
}


package c3po.ned;

simple TL_Router extends TL_OJF_MWM
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

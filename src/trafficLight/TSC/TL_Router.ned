
package VENTOS.src.trafficLight.TSC;

simple TL_Router extends TL_OJFM
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

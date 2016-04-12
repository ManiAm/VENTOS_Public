
package VENTOS.src.trafficLight.TSC;

simple TL_Router extends TL_OJFM
{
    parameters:
        @class(VENTOS::TrafficLightRouter);
        @display("i=block/network2");  
        
        // 0=no TLControl, 1=HighDensity, 2=LowDensity 3=Cooperative
        int TLLogicMode = default(0);
        
        double HighDensityRecalculateFrequency = default(30);
        double LowDensityExtendTime = default(3);
        double MaxPhaseDuration = default(60);  // Low density/Cooperative will not extend traffic lights to last longer than this  
        double MinPhaseDuration = default(10);  // Cooperative control will not assign a phase duration shorter than this
}

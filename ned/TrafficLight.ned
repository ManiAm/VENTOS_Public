
package c3po.ned;

simple TrafficLight
{
    parameters:
        @class(VENTOS::TrafficLight);
        @display("i=block/network2");  
        
        bool UseTLLogic;
        bool UseHighDensityLogic;
        double HighDensityRecalculateFrequency;
        double LowDensityExtendTime;
        
        
}

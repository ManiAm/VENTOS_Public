
package c3po.ned;

simple TrafficLightControl extends TrafficLightBase
{
    parameters:
        @class(VENTOS::TrafficLightControl);
        @display("i=block/network2");  
        
        int TLControlMode;
        
        double minGreenTime;
        double maxGreenTime;
        double yellowTime;
        double redTime;
        double passageTime;        
        double detectorPos;
}

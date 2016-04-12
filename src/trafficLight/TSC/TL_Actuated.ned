
package VENTOS.src.trafficLight.TSC;

simple TL_Actuated extends TL_Webster
{
    parameters:
        @class(VENTOS::TrafficLightActuated);
        @display("i=block/network2");        
        
        bool greenExtension = default(true);
        
        // -1 : passage time is calculated precisely for each incoming lane (might not be realistic!)
        // > 0: value set by the user (2.0s gives good results if LD is at 35m distance)
        double passageTime = default(-1s) @unit(s);
}

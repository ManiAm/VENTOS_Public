
package c3po.application.trafficLight.TSC;

simple TL_Actuated extends TL_Webster
{
    parameters:
        @class(VENTOS::TrafficLightActuated);
        @display("i=block/network2");        
        
        bool greenExtension = default(true);
        double passageTime = default(0s) @unit(s);
}

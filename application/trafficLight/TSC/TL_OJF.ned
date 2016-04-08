
package c3po.application.trafficLight.TSC;

simple TL_OJF extends TL_LQF
{
    parameters:
        @class(VENTOS::TrafficLightOJF);
        @display("i=block/network2");              
}

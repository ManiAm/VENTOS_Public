
package c3po.application.trafficLight;

simple TL_OJF extends TL_LongestQueue
{
    parameters:
        @class(VENTOS::TrafficLightOJF);
        @display("i=block/network2");              
}

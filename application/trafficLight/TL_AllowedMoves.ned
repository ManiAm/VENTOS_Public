
package c3po.application.trafficLight;

simple TL_AllowedMoves extends IntersectionDelay
{
    parameters:
        @class(VENTOS::TrafficLightAllowedMoves);
        @display("i=block/network2");        
}

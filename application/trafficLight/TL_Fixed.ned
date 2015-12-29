
package c3po.application.trafficLight;

simple TL_Fixed extends TL_AllowedMoves
{
    parameters:
        @class(VENTOS::TrafficLightFixed);
        @display("i=block/network2");        
}

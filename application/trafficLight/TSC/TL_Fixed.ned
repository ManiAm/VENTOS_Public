
package c3po.application.trafficLight.TSC;
import c3po.application.trafficLight.TL_AllowedMoves;

simple TL_Fixed extends TL_AllowedMoves
{
    parameters:
        @class(VENTOS::TrafficLightFixed);
        @display("i=block/network2");        
}

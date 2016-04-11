
package VENTOS.src.trafficLight.TSC;
import VENTOS.src.trafficLight.TL_AllowedMoves;

simple TL_Fixed extends TL_AllowedMoves
{
    parameters:
        @class(VENTOS::TrafficLightFixed);
        @display("i=block/network2");        
}

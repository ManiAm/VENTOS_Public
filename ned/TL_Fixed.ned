
package c3po.ned;

simple TL_Fixed extends TL_AllowedMoves
{
    parameters:
        @class(VENTOS::TrafficLightFixed);
        @display("i=block/network2");        
}

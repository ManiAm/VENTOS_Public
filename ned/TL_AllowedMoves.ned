
package c3po.ned;

simple TL_AllowedMoves extends IntersectionDelay
{
    parameters:
        @class(VENTOS::TrafficLightAllowedMoves);
        @display("i=block/network2");        
}

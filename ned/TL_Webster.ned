
package c3po.ned;

simple TL_Webster extends TL_Fixed
{
    parameters:
        @class(VENTOS::TrafficLightWebster);
        @display("i=block/network2");    
        
        double alpha;          
}


package VENTOS.src.trafficLight;

simple TL_LoopDetectors extends TL_Base
{
    parameters:
        @class(VENTOS::LoopDetectors);
        @display("i=block/network2");
        
        bool collectInductionLoopData; 
}


package c3po.ned;

simple TL_LoopDetectors extends TL_Base
{
    parameters:
        @class(VENTOS::LoopDetectors);
        @display("i=block/network2");  
        
        bool measureIntersectionQueue;
        bool measureTrafficDemand;
        
        bool collectInductionLoopData; 
        bool collectTLQueuingData;
        bool collectTLPhasingData; 
}

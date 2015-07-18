
package c3po.ned;

simple TL_LoopDetectors extends TL_Base
{
    parameters:
        @class(VENTOS::LoopDetectors);
        @display("i=block/network2");  
        
        double minGreenTime = default(0s) @unit(s);
        double maxGreenTime = default(0s) @unit(s);
        double yellowTime = default(0s) @unit(s);
        double redTime = default(0s) @unit(s);
        double maxCycleLength = default(120s) @unit(s);  
        
        bool measureIntersectionQueue;
        bool measureTrafficDemand;        
        int trafficDemandBuffSize;
        
        bool collectInductionLoopData; 
        bool collectTLQueuingData;
        bool collectTLPhasingData; 
}

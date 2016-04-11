
package VENTOS.src.trafficLight;

simple TL_MeasureTrafficParams extends TL_LoopDetectors
{
    parameters:
        @class(VENTOS::MeasureTrafficParams);
        @display("i=block/network2"); 
        
        bool measureIntersectionQueue;
        bool measureTrafficDemand;  
        int measureTrafficDemandMode;      
        int trafficDemandBuffSize;
        
        bool collectTLQueuingData;
        bool collectTLPhasingData; 
}

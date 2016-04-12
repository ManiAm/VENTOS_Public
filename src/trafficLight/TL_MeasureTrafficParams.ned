
package VENTOS.src.trafficLight;

simple TL_MeasureTrafficParams extends TL_LoopDetectors
{
    parameters:
        @class(VENTOS::MeasureTrafficParams);
        @display("i=block/network2"); 
        
        bool measureIntersectionQueue = default(false);
        bool measureTrafficDemand = default(false);  
        int measureTrafficDemandMode = default(2); // =1: measure headway time  =2: measure total # of vehicles in a period (more accurate)
        int trafficDemandBuffSize = default(10);  // buffer size for storing traffic demands
        
        bool collectTLQueuingData = default(false);
        bool collectTLPhasingData = default(false); 
}

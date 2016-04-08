
package c3po.application.trafficLight;

simple TL_TLStateRecord extends TL_MeasureTrafficParams
{
    parameters:
        @class(VENTOS::TLStateRecord);
        @display("i=block/network2");  
        
        double minGreenTime = default(0s) @unit(s);
        double maxGreenTime = default(0s) @unit(s);
        double yellowTime = default(0s) @unit(s);
        double redTime = default(0s) @unit(s);
        double maxCycleLength = default(120s) @unit(s);  
}

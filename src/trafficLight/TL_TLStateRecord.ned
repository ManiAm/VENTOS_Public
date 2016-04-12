
package VENTOS.src.trafficLight;

simple TL_TLStateRecord extends TL_MeasureTrafficParams
{
    parameters:
        @class(VENTOS::TLStateRecord);
        @display("i=block/network2");  
        
        double minGreenTime = default(8.3s) @unit(s);  // long enough to clear the queue (5 vehicles)
        double maxGreenTime = default(20.0s) @unit(s); // about twice as minGreenTime --should not exceed green time in fix-time (bigger value negatively affect performance!)
        double yellowTime = default(4.0s) @unit(s);    // yellow time should be >= 3s
        double redTime = default(2.0s) @unit(s);
        double maxCycleLength = default(120s) @unit(s);  
}

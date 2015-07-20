
package c3po.ned;

simple AddEntity
{
    parameters:
        @class(VENTOS::AddEntity);
        @display("i=block/network2");
        
        bool on = default(true);
        int mode;
        
        int totalVehicles;
        string vehiclesType;
        
        int lambda;
        int plnSize;
        double plnSpace = default(3.5s) @unit(s);
        double overlap;
        string routeDist;
}


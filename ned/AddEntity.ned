
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
        int distribution;
        int interval = default(1000ms) @unit(ms);        
        int lambda;
        
        int plnSize;
        double plnSpace = default(3.5s) @unit(s);
        double overlap;
        string vehRouteDist;
        bool vehMultiClass;
        string vehClassDist;
        bool bike;
        string bikeRouteDist;
}


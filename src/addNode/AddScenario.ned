
package VENTOS.src.addNode;

simple AddScenario extends AddNode
{
    parameters:
        @class(VENTOS::AddScenario);
        @display("i=block/network2");
        
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


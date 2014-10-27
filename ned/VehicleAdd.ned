
package c3po.ned;


simple VehicleAdd
{
    parameters:
        @class(VENTOS::VehicleAdd);
        @display("i=block/network2");
        
        bool on = default(true);
        int mode;
        int totalVehicles;
        int lambda;
        int plnSize;
        double plnSpace = default(3.5s) @unit(s);
}


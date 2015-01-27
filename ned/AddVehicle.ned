
package c3po.ned;

simple AddVehicle
{
    parameters:
        @class(VENTOS::AddVehicle);
        @display("i=block/network2");
        
        bool on = default(true);
        int mode;
        int totalVehicles;
        int lambda;
        int plnSize;
        double plnSpace = default(3.5s) @unit(s);
}



package c3po.application.global;

simple Statistics
{
    parameters:
        @class(VENTOS::Statistics);
        
        bool collectVehiclesData;
        int vehicleDataLevel;
        
        bool reportMAClayerData;
        bool reportPlnManagerData;
        bool reportBeaconsData;
}

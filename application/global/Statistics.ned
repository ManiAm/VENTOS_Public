
package c3po.application.global;

simple Statistics
{
    parameters:
        @class(VENTOS::Statistics);
        
        bool collectVehiclesData;
        int vehicleDataLevel;
        
        bool reportPlnManagerData;
        bool reportBeaconsData;
}

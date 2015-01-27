
package c3po.ned;

simple ApplPedManager extends ApplPedBeacon
{
    parameters:
        @class(VENTOS::ApplPedManager);
        @display("i=block/app2");
        
        bool SUMOvehicleDebug = default(false);
}

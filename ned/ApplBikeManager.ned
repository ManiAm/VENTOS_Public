
package c3po.ned;

simple ApplBikeManager extends ApplBikeBeacon
{
    parameters:
        @class(VENTOS::ApplBikeManager);
        @display("i=block/app2");
        
        bool SUMOvehicleDebug = default(false);
}

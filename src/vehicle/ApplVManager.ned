
package VENTOS.src.vehicle;

import VENTOS.src.vehicle.platoon.ApplVPlatoonMg;

simple ApplVManager extends ApplVPlatoonMg
{
    parameters:
        @class(VENTOS::ApplVManager);
        @display("i=block/app2");
        
        bool degradeToACC = default(true);
        bool SUMOvehicleDebug = default(false);
        
        double droppT = default(0s) @unit(s);
        string droppV = default("");
        double plr = default(0);
        
        bool measurementError;
        double errorGap;
        double errorRelSpeed;
}

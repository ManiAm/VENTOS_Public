
package VENTOS.src.vehicle;

import VENTOS.src.vehicle.platoon.ApplVPlatoonMg;

simple ApplVManager extends ApplVPlatoonMg
{
    parameters:
        @class(VENTOS::ApplVManager);
        @display("i=block/app2");
        
        bool SUMOvehicleDebug = default(false);  // controling SUMO debug output for "each vehicle"
        bool degradeToACC = default(true);
        
        // beacon loss ratio in application layer
        double plr = default(0);  // packet loss ratio (percent) in each simulation timeStep
        double droppT = default(30s) @unit(s);
        string droppV = default("");  // plr for which vehicle? empty string means all
        
        // measurement error
        bool measurementError = default(false);
        double errorGap = default(0.01);
        double errorRelSpeed = default(0.05);
}

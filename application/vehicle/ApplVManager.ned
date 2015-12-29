
package c3po.application.vehicle;

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

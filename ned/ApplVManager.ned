
package c3po.ned;


simple ApplVManager extends ApplVPlatoonMg
{
    parameters:
        @class(VENTOS::ApplVManager);
        @display("i=block/app2");
        
        int controllerType = default(0);
        bool degradeToACC = default(true);
        bool SUMOvehicleDebug = default(false);
        
        double droppT = default(0s) @unit(s);
        string droppV = default("");
        double plr = default(0);
        
        bool measurementError;
        double errorGap;
        double errorRelSpeed;
}

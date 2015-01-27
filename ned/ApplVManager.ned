
package c3po.ned;


simple ApplVManager extends ApplVPlatoonMg
{
    parameters:
        @class(VENTOS::ApplVManager);
        @display("i=block/app2");
        
        double droppT = default(0s) @unit(s);
        string droppV = default("");
        double plr = default(0);
        
        bool SUMOvehicleDebug = default(false);
        bool modeSwitch = default(true);
        
        bool measurementError;
        double errorGap;
        double errorRelSpeed;
}

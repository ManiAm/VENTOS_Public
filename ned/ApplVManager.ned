package c3po.ned;

simple ApplVManager extends ApplVPlatoonMg
{
    parameters:
        @class(ApplVManager);
        @display("i=block/app2");
        
        double sonarDist = default(900);
        
        double droppT = default(0s) @unit(s);
        string droppV = default("");
        double plr = default(0);
        
        bool SUMOvehicleDebug = default(false);
        bool modeSwitch = default(true);
        
        bool measurementError;
        double errorGap;
        double errorRelSpeed;
        
        bool one_vehicle_look_ahead = default(false);
}

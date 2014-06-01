package c3po.ned;

simple ApplVSumoInteraction extends ApplVPlatoonFormation2
{
    parameters:
        @class(ApplVSumoInteraction);
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

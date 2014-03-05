package c3po.ned;

simple ApplVSumoInteraction extends ApplVPlatoonFormation
{
    parameters:
        @class(ApplVSumoInteraction);
        @display("i=block/app2");
        
        double droppT = default(0s) @unit(s);
        string droppV = default("");
        double plr = default(0);
        
        bool modeSwitch = default(true);
}

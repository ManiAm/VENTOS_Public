package c3po.ned;

simple ApplVPlatoon extends ApplVBeacon
{
    parameters:
        @class(ApplVPlatoon);
        @display("i=block/app2");
        
        bool one_vehicle_look_ahead = default(false);                
        string platoonID;
}

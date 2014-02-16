package c3po.ned;

simple ApplVBeaconPlatoonLeader extends ApplVBeacon
{
    parameters:
        @class(ApplVBeaconPlatoonLeader);
        @display("i=block/app2");
                
        bool one_vehicle_look_ahead = default(false);
        string platoonLeaderID;
        int platoonID;
}

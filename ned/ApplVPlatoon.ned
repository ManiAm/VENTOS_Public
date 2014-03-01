package c3po.ned;

simple ApplVPlatoon extends ApplVBeaconPlatoonLeader
{
    parameters:
        @class(ApplVPlatoon);
        @display("i=block/app2");
        
        bool platoonFormation = default(false);  //whether this applayer should send data
        int maxPlatoonSize;
        
        int dataLengthBits = default(1024bit) @unit(bit); //the length of a data packet
        bool dataOnSch = default(true); //tells the applayer whether to use a service channel for datapackets or the control channel
        int dataPriority = default(3); //the access category for data packets
}

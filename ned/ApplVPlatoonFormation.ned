package c3po.ned;

simple ApplVPlatoonFormation extends ApplVPlatoon
{
    parameters:
        @class(ApplVPlatoonFormation);
        @display("i=block/app2");
        
        bool platoonFormation = default(false);  //whether this applayer should send data
        int maxPlatoonSize;
        double timer1Value = default(1s) @unit(s);
        double timer2Value = default(1s) @unit(s);
        
        int dataLengthBits = default(1024bit) @unit(bit); //the length of a data packet
        bool dataOnSch = default(true); //tells the applayer whether to use a service channel for datapackets or the control channel
        int dataPriority = default(3); //the access category for data packets
}

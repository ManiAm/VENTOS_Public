package c3po.ned;

simple ApplVPlatoonFormation extends ApplVManager
{
    parameters:
        @class(ApplVPlatoonFormation);
        @display("i=block/app2");
        
        bool platoonFormation = default(false);  //whether this applayer should send data
        int maxPlatoonSize;
        double timer1Value = default(1s) @unit(s);
        double timer2Value = default(1s) @unit(s);
}

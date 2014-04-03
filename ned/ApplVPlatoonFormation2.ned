package c3po.ned;

simple ApplVPlatoonFormation2 extends ApplVPlatoonFormation
{
    parameters:
        @class(ApplVPlatoonFormation2);
        @display("i=block/app2");
        
        bool platoonLeaderLeave = true; 
        bool platoonMemberLeave = true;   
        double timer3Value = default(1s) @unit(s);  
}

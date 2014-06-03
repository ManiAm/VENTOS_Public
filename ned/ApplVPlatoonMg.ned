package c3po.ned;


simple ApplVPlatoonMg extends ApplV_AID
{
    parameters:
        @class(ApplVPlatoonMg); 
        
        int maxPlatoonSize;
        double timer1Value = default(1s) @unit(s);
        double timer2Value = default(1s) @unit(s);
        
        bool platoonLeaderLeave = true; 
        bool platoonMemberLeave = true;   
        double timer3Value = default(1s) @unit(s);
}

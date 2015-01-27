
package c3po.ned;


simple ApplVPlatoonMg extends ApplVPlatoonFormed
{
    parameters:
        @class(VENTOS::ApplVPlatoonMg); 
        
        int maxPlatoonSize;
        int optPlatoonSize;
        
        bool adaptiveTG;
        double TP = default(3.5s) @unit(s);
        double TG1 = default(0.55s) @unit(s);
        double TG2 = default(0.6s) @unit(s);
        
        bool entryEnabled;
        bool mergeEnabled;
        bool splitEnabled;  
        bool followerLeaveEnabled;
        bool leaderLeaveEnabled;   
        
        int coordinationMode = default(-1);
}

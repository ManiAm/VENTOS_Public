package c3po.ned;


simple ApplVPlatoonMg extends ApplVPlatoonFormed
{
    parameters:
        @class(VENTOS::ApplVPlatoonMg); 
        
        int maxPlatoonSize;
        int optPlatoonSize;
        
        bool mergeEnabled;
        bool splitEnabled;  
        bool followerLeaveEnabled;
        bool leaderLeaveEnabled; 
        
        double mergeGap;
        double splitGap;     
}

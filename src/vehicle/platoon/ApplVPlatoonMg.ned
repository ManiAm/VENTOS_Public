
package VENTOS.src.vehicle.platoon;

simple ApplVPlatoonMg extends ApplVPlatoonFormed
{
    parameters:
        @class(VENTOS::ApplVPlatoonMg); 
        
        int maxPlatoonSize = default(10);
        int optPlatoonSize = default(5);
        
        bool adaptiveTG = default(true);
        double TP = default(3.5s) @unit(s);   // intra-platoon time-gap (between platoons)
        double TG1 = default(0.55s) @unit(s); // inter-platoon time-gap 1 (between followers)
        double TG2 = default(0.6s) @unit(s);  // inter-platoon time-gap 2 (between followers)
        
        int coordinationMode = default(-1);
        
        bool entryEnabled = default(true);
        bool mergeEnabled = default(true);
        bool splitEnabled = default(true);
        bool followerLeaveEnabled = default(true);
        bool leaderLeaveEnabled = default(true);   
}

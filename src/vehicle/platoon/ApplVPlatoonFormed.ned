
package VENTOS.src.vehicle.platoon;
import VENTOS.src.vehicle.ApplVAID;

simple ApplVPlatoonFormed extends ApplVAID
{
    parameters:
        @class(VENTOS::ApplVPlatoonFormed); 
        
        // plnMode = 1: no platooning
        // plnMode = 2: pre-defined platoon
        // plnMode = 3: platoon management is active
        int plnMode = default(1);
        
        string preDefinedPlatoonID = default("CACC1"); // only in plnMode = 2
}

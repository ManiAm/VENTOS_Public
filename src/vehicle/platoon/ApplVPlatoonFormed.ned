
package VENTOS.src.vehicle.platoon;
import VENTOS.src.vehicle.ApplVAID;

simple ApplVPlatoonFormed extends ApplVAID
{
    parameters:
        @class(VENTOS::ApplVPlatoonFormed); 
        
        int plnMode = default(0);
        string preDefinedPlatoonID;        
}

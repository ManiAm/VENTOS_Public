
package c3po.application.vehicle.platoon;
import c3po.application.vehicle.ApplVAID;

simple ApplVPlatoonFormed extends ApplVAID
{
    parameters:
        @class(VENTOS::ApplVPlatoonFormed); 
        
        int plnMode = default(0);
        string preDefinedPlatoonID;        
}

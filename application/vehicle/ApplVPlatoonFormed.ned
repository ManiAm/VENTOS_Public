
package c3po.application.vehicle;

simple ApplVPlatoonFormed extends ApplVAID
{
    parameters:
        @class(VENTOS::ApplVPlatoonFormed); 
        
        int plnMode = default(0);
        string preDefinedPlatoonID;        
}

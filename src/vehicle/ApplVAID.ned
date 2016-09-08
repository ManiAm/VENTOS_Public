
package VENTOS.src.vehicle;

simple ApplVAID extends ApplVDynamicRouting
{
    parameters:
        @class(VENTOS::ApplVAID);       
        
        // Automatic Incident Detection (AID)
        bool AID = default(false);
}

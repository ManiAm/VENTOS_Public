
package VENTOS.src.vehicle;

simple ApplVRedpine extends ApplVAID
{
    parameters:
        @class(VENTOS::ApplVRedpine);       
        
        bool hardBreakingDetection = default(false);
        bool EEBL = default(false);
}

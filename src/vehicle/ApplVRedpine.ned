
package VENTOS.src.vehicle;

simple ApplVRedpine extends ApplVAID
{
    parameters:
        @class(VENTOS::ApplVRedpine);       
        
        bool hardBreakDetection = default(false);
        bool EEBL = default(false);
        
        bool print_HIL2Sim = default(false);
        bool print_Sim2HIL = default(false);
}

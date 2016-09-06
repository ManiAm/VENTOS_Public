
package VENTOS.src.vehicle;

simple ApplVOBU extends ApplVAID
{
    parameters:
        @class(VENTOS::ApplVOBU);       
        
        bool forwardCollisionDetection = default(false);
        bool EEBL_Active = default(false);
        
        bool print_HIL2Sim = default(false);
        bool print_Sim2HIL = default(false);
}

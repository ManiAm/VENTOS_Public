
package c3po.application.vehicle;

simple ApplVAID extends ApplVSystem
{
    parameters:
        @class(VENTOS::ApplVAID);       
                     
        bool AID = default(false);
}

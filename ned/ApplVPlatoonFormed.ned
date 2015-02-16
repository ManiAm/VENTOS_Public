
package c3po.ned;


simple ApplVPlatoonFormed extends ApplVAID
{
    parameters:
        @class(VENTOS::ApplVPlatoonFormed); 
        
        int plnMode = default(0);
        string preDefinedPlatoonID;        
}

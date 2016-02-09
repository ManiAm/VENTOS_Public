
package c3po.application.rsu;

simple ApplRSUTLVANET extends ApplRSUAID
{
    parameters:
        @class(VENTOS::ApplRSUTLVANET);   
        
        bool monitorVehApproach;
        bool collectVehApproach;
        
        bool classifier;
        bool collectTrainingData;
}

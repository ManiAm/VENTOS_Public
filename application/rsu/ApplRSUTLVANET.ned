
package c3po.application.rsu;

simple ApplRSUTLVANET extends ApplRSUAID
{
    parameters:
        @class(VENTOS::ApplRSUTLVANET);   
        
        bool monitorVehApproach;
        bool collectVehApproach;
        
        bool classifier;
        double trainError @unit("m") = default(0m);
        double GPSerror @unit("m") = default(0m);
}

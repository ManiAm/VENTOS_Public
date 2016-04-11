
package VENTOS.src.rsu;

simple ApplRSUClassify extends ApplRSUMonitor
{
    parameters:
        @class(VENTOS::ApplRSUCLASSIFY);   
        
        bool classifier;
        double trainError @unit("m") = default(0m);
        double GPSerror @unit("m") = default(0m);
}

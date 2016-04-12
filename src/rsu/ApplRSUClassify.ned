
package VENTOS.src.rsu;

simple ApplRSUClassify extends ApplRSUMonitor
{
    parameters:
        @class(VENTOS::ApplRSUCLASSIFY);   
        
        bool classifier = default(false);
        double trainError @unit("m") = default(0m);  // adding error to training data
        double GPSerror @unit("m") = default(0m);   // adding error to received beacon
}

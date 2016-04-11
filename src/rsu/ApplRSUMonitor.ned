
package VENTOS.src.rsu;

simple ApplRSUMonitor extends ApplRSUBase
{
    parameters:
        @class(VENTOS::ApplRSUMonitor);   
        
        bool activeDetection;  
        bool collectVehApproach;
}

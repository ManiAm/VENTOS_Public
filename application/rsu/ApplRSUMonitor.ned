
package c3po.application.rsu;

simple ApplRSUMonitor extends ApplRSUBase
{
    parameters:
        @class(VENTOS::ApplRSUMonitor);   
        
        bool activeDetection;  
        bool collectVehApproach;
}

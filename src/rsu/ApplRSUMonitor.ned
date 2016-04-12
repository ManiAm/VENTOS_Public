
package VENTOS.src.rsu;

simple ApplRSUMonitor extends ApplRSUBase
{
    parameters:
        @class(VENTOS::ApplRSUMonitor);   
        
        bool activeDetection = default(false);  // if RSU should monitor the intersection activity?
        bool collectVehApproach = default(false); // only if activeDetection = true
}


package VENTOS.src.rsu;

simple ApplRSUAID extends ApplRSUClassify
{
    parameters:
        @class(VENTOS::ApplRSUAID);
        
        bool enableAID;
        bool printIncidentDetection;
}

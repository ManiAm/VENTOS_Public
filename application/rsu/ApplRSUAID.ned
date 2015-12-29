
package c3po.application.rsu;

simple ApplRSUAID extends ApplRSUBase
{
    parameters:
        @class(VENTOS::ApplRSUAID);
        
        bool enableAID;
        bool printIncidentDetection;
}

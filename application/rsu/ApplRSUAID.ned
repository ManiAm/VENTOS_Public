
package c3po.application.rsu;

simple ApplRSUAID extends ApplRSUClassify
{
    parameters:
        @class(VENTOS::ApplRSUAID);
        
        bool enableAID;
        bool printIncidentDetection;
}

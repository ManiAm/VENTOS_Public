
package VENTOS.src.rsu;

simple ApplRSUCRL extends ApplRSUAID
{
    parameters:
        @class(VENTOS::ApplRSUCRL);
        
        int CRLdistAlg;        
        double CRL_Interval @unit(s);
        double beacon_Interval @unit(s);
        double I2V_tho @unit(s);
}

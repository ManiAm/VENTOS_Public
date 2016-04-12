
package VENTOS.src.rsu;

simple ApplRSUCRL extends ApplRSUAID
{
    parameters:
        @class(VENTOS::ApplRSUCRL);
        
        // CRLdistAlg=-1: Not active
        // CRLdistAlg= 0: CRL_RSU_Only
        // CRLdistAlg= 1: CRL_C2C_Epidemic
        // CRLdistAlg= 2: CRL_MPB
        // CRLdistAlg= 3: CRL_ICE
        // CRLdistAlg= 4: CRL_ICEv2

        // CRLdistAlg= 5: CRL_Broadcast           -->  V2V communication: when a node recieves a broadcast beacon, it sends its pieces.
        // CRLdistAlg= 6: CRL_BBroadcast          -->  V2V communication: nodes send their pieces to each other.
        // CRLdistAlg= 7: CRL_C2C_Epidemic_Ideal  --> No actual usage of IEEE 802.11g frames
        // CRLdistAlg= 8: CRL_NewMPB
        int CRLdistAlg = default(-1); 
               
        double CRL_Interval = default(100s) @unit(s);
        double beacon_Interval = default(1s) @unit(s);
        
        // tho: maximum time devoted to CRL broadcasting (=0: No time constraints)
        // frame size (bits) = 18624, bitrate (bps) = 2e+06 --> TxTime (s) = 0.009312
        // ex. if tho=0.03s then only 3 CRLs can be sent.
        // ex. if tho=0.1s then only 10 CRLs can be sent.
        double I2V_tho = default(0.1s) @unit(s);
}

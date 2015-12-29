
package c3po.application.sniffing;

simple SniffEther
{
    parameters:
        @class(VENTOS::SniffEthernet);
        @display("i=block/network2");
        
        bool on = default(false);
        string interface;
        string filter_exp;
        bool printCaptured;
        bool printDataPayload;
        bool printStat;
}


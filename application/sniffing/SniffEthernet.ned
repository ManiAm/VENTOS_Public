
package c3po.application.sniffing;

simple SniffEthernet
{
    parameters:
        @class(VENTOS::SniffEthernet);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        bool on = default(false);
        string interface;
        string filter_exp;
        bool printCaptured;
        bool printDataPayload;
        bool printStat;
}


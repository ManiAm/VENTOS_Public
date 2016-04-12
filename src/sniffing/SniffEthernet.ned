
package VENTOS.src.sniffing;

simple SniffEthernet
{
    parameters:
        @class(VENTOS::SniffEthernet);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        bool on = default(false);
        
        string interface = default("eth0");
        string filter_exp = default("ip");
        bool printCaptured = default(false);
        bool printDataPayload = default(false);
        bool printStat = default(false);
}


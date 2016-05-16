
package VENTOS.src.interfacing;

simple Ethernet
{
    parameters:
        @class(VENTOS::Ethernet);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        bool active = default(false);
        
        string interface = default("eth0");
        string filter_exp = default("ip");
        bool printCaptured = default(false);
        bool printDataPayload = default(false);
        bool printStat = default(false);
}


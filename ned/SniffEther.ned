
package c3po.ned;

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
}


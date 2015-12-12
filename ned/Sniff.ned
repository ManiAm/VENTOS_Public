
package c3po.ned;

simple Sniff
{
    parameters:
        @class(VENTOS::SniffEthernet);
        @display("i=block/network2");
        
        bool on = default(false);
}



package c3po.application.sniffing;

simple SniffBluetooth
{
    parameters:
        @class(VENTOS::SniffBluetooth);
        @display("i=block/network2");
        
        bool on = default(false);
        string interface;
}


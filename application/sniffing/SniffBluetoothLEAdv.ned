
package c3po.application.sniffing;

simple SniffBluetoothLEAdv extends SniffBluetoothLE
{
    parameters:
        @class(VENTOS::SniffBluetoothLEAdv);
        @display("i=block/network2");
        
        bool advertisement = default(false);
}


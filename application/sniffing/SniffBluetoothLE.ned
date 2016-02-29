
package c3po.application.sniffing;

simple SniffBluetoothLE extends SniffBluetooth
{
    parameters:
        @class(VENTOS::SniffBluetoothLE);
        @display("i=block/network2");
        
        bool LEBTon = default(false);
}


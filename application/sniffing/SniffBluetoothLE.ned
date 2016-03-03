
package c3po.application.sniffing;

simple SniffBluetoothLE extends SniffBluetooth
{
    parameters:
        @class(VENTOS::SniffBluetoothLE);
        @display("i=block/network2");
        
        bool BLEon = default(false);
        int BLEscanDeviceID;
        int scanTime = default(10s) @unit(s);
}


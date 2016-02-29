
package c3po.application.sniffing;

simple SniffBluetoothDump extends AdvertiseBeacon
{
    parameters:
        @class(VENTOS::SniffBluetoothDump);
        @display("i=block/network2");
        
        bool dumpOn = default(false);
}


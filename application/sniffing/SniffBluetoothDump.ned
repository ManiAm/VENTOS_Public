
package c3po.application.sniffing;

simple SniffBluetoothDump extends AdvertiseBeacon
{
    parameters:
        @class(VENTOS::SniffBluetoothDump);
        @display("i=block/network2");
        
        bool dump_On = default(false);
        int BLE_dump_deviceID;
        int sniff_time = default(-1s) @unit(s);
}


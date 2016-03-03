
package c3po.application.sniffing;

simple SniffBluetooth
{
    parameters:
        @class(VENTOS::SniffBluetooth);
        @display("i=block/network2");
        
        bool BT_on = default(false);
        int BT_scan_deviceID;        
        int BT_scan_length = default(8);
}



package VENTOS.src.sniffing;

simple SniffBluetooth
{
    parameters:
        @class(VENTOS::SniffBluetooth);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); //header length of the application
        
        bool listLocalDevices = default(false);
                
        bool BT_on = default(false);
        int BT_scan_deviceID = default(-1);        
        int BT_scan_length = default(8);
}


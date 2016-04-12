
package VENTOS.src.sniffing;

simple SniffBluetoothLE extends SniffBluetooth
{
    parameters:
        @class(VENTOS::SniffBluetoothLE);
        @display("i=block/network2");
        
        bool BLE_on = default(false);
        int BLE_scan_deviceID = default(-1);
        int BLE_scan_time = default(10s) @unit(s);
        
        // BLE parameters: check page 973 (1255 of document) of BT spec 4.1

        // 0x00: Passive scanning. No SCAN_REQ packets shall be sent
        // 0x01: Active scanning. SCAN_REQ packets may be sent
        int BLE_scan_type = default(0x01);
        
        // Time interval from when the Controller started its last LE scan 
        // until it begins the subsequent LE scan.
        // Range is 0x0004 to 0x4000 and Time is N * 0.625 msec
        int BLE_interval = default(0x0010);
        
        // The duration of the LE scan. LE_Scan_Window shall be less than
        // or equal to LE_Scan_Interval. Range is 0x0004 to 0x4000 and
        // Time is N * 0.625 msec
        int BLE_window = default(0x0010);
        
        // 0: Public Device Address
        // 1: Random Device Address
        int BLE_own_type = default(0x00);
        
        // 0: Accept all advertisement packets. Directed advertising packets 
        // which are not addressed for this device shall be ignored.
        // 1: Ignore advertisement packets from devices not in the White List Only.
        // Directed advertising packets which are not addressed for this device
        // shall be ignored.
        int BLE_filter_policy = default(0x00);       
}


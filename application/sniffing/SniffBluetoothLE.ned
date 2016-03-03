
package c3po.application.sniffing;

simple SniffBluetoothLE extends SniffBluetooth
{
    parameters:
        @class(VENTOS::SniffBluetoothLE);
        @display("i=block/network2");
        
        bool BLE_on = default(false);
        
        int BLE_scan_type;
        int BLE_interval;
        int BLE_window;
        int BLE_own_type;
        int BLE_filter_policy;       
        int BLE_scan_deviceID;
        int BLE_scan_time = default(10s) @unit(s);
}


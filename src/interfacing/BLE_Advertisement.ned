
package VENTOS.src.interfacing;

simple BLE_Advertisement extends BLE
{
    parameters:
        @class(VENTOS::BLE_Advertisement);
        @display("i=block/network2");
       
        bool BLE_advertisement = default(false);        
        int BLE_adv_deviceID = default(-1);     
        
        // Advertisement parameters: check page 965 (1247 of document) of BT spec 4.1
        
        // Minimum and Maximum advertising interval for undirected and low duty cycle directed advertising.
        // Range is 0x0020 to 0x4000 and Time is  N * 0.625 msec
        int BLE_minInterval = default(0x0800);
        int BLE_maxInterval = default(0x0800);
        
        // ADV_IND         0x00    Connectable undirected advertising
        // ADV_DIRECT_IND  0x01    Connectable high duty cycle directed advertising
        // ADV_SCAN_IND    0x02    Scannable undirected advertising
        // ADV_NONCONN_IND 0x03    Non connectable undirected advertising
        // ADV_SCAN_RSP    0x04
        int BLE_adv_type = default(3);
        
        // 1: enable channel 37 
        // 2: enable channel 38
        // 4: enable channel 39
        // 7: all channels are enabled
        int BLE_channelNumber = default(7);  
        
        // 0:iBeacon, 1:AltBeacon
        int BLE_beaconType = default(0);
        
        // iBeacon parameters
        string iBeacon_UUID = default("");
        string iBeacon_major = default("");
        string iBeacon_minor = default("");
        string iBeacon_TXpower = default("");
        
        // AltBeacon parameters
        string AltBeacon_MFGID = default("");
        string AltBeacon_beaconID = default("");
        string AltBeacon_refRSSI = default("");
        string AltBeacon_MFGRSVD = default("");
}


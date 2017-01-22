package VENTOS.src.interfacing;

simple Ethernet
{
    parameters:
        @class(VENTOS::Ethernet);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); // header length of the application 
        bool active = default(false);
        
        string interface = default("eth0");
        string filter_exp = default("ip");
        
        bool printStat = default(false);         // print capturing statistics: # of frames, # timeOuts, etc.
        bool printCaptured = default(false);     // print the header of captured packets 
        bool printDataPayload = default(false);  // print the data payload -- TCP, UDP, ICMP
}


simple USB
{
    parameters:
        @class(VENTOS::USB);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); // header length of the application 
        bool active = default(false);
        
        bool listUSBdevices = default(false);
        bool listUSBdevicesDetailed = default(false);
        bool hotPlug = default(false);
        
        int target_vendor_id = default(0x0);
        int target_product_id = default(0x0);
        int target_interfaceNumber = default(0);
        int target_interruptEP = default(0x80);
}


simple Bluetooth
{
    parameters:
        @class(VENTOS::Bluetooth);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); // header length of the application
        
        bool listLocalDevices = default(false);
        
        bool BT_on = default(false);
        int BT_scan_deviceID = default(-1);        
        int BT_scan_length = default(8);
}


simple BLE extends Bluetooth
{
    parameters:
        @class(VENTOS::BLE);
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


simple BLE_Dump extends BLE_Advertisement
{
    parameters:
        @class(VENTOS::BLE_Dump);
        @display("i=block/network2");
        
        bool dump_On = default(false);
        int BLE_dump_deviceID = default(-1);
        int sniff_time = default(-1s) @unit(s);
}


package c3po.application.sniffing;

simple AdvertiseBeacon extends SniffBluetoothLE
{
    parameters:
        @class(VENTOS::AdvertiseBeacon);
        @display("i=block/network2");
        
        bool BLE_advertisement = default(false);
        
        int BLE_adv_deviceID;        
        int BLE_minInterval;
        int BLE_maxInterval;
        int BLE_adv_type;
        int BLE_channelNumber;        
        int BLE_beaconType;
        
        string iBeacon_UUID;
        string iBeacon_major;
        string iBeacon_minor;
        string iBeacon_TXpower;
        
        string AltBeacon_MFGID;
        string AltBeacon_beaconID;
        string AltBeacon_refRSSI;
        string AltBeacon_MFGRSVD;
}


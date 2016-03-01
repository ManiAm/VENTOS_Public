
package c3po.application.sniffing;

simple AdvertiseBeacon extends SniffBluetoothLE
{
    parameters:
        @class(VENTOS::AdvertiseBeacon);
        @display("i=block/network2");
        
        bool advertisement = default(false);
        int beaconType;
        
        string iBeacon_UUID;
        string iBeacon_major;
        string iBeacon_minor;
        string iBeacon_TXpower;
        
        string AltBeacon_MFGID;
        string AltBeacon_beaconID;
        string AltBeacon_refRSSI;
        string AltBeacon_MFGRSVD;
}


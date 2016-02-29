
package c3po.application.sniffing;

simple AdvertiseBeacon extends SniffBluetoothLE
{
    parameters:
        @class(VENTOS::AdvertiseBeacon);
        @display("i=block/network2");
        
        bool advertisement = default(false);
        int beaconType;
        
        string UUID;
        string major;
        string minor;
        string TXpower;
}


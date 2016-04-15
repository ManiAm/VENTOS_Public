
package VENTOS.src.interfacing;

simple BLE_Dump extends BLE_Advertisement
{
    parameters:
        @class(VENTOS::BLE_Dump);
        @display("i=block/network2");
        
        bool dump_On = default(false);
        int BLE_dump_deviceID = default(-1);
        int sniff_time = default(-1s) @unit(s);
}



package c3po.ned;

simple SniffUSB
{
    parameters:
        @class(VENTOS::SniffUSB);
        @display("i=block/network2");
        
        bool on = default(false);
        bool listUSBdevices;
        bool listUSBdevicesDetailed;
        bool hotPlug;
        
        int target_vendor_id;
		int target_product_id;
		int target_interfaceNumber;
		int target_interruptEP;
}


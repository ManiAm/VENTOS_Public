
package VENTOS.src.sniffing;

simple SniffUSB
{
    parameters:
        @class(VENTOS::SniffUSB);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        bool on = default(false);
        bool listUSBdevices;
        bool listUSBdevicesDetailed;
        bool hotPlug;
        
        int target_vendor_id;
		int target_product_id;
		int target_interfaceNumber;
		int target_interruptEP;
}


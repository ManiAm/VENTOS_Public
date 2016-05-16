
package VENTOS.src.interfacing;

simple USB
{
    parameters:
        @class(VENTOS::USB);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        bool active = default(false);
        
        bool listUSBdevices = default(false);
        bool listUSBdevicesDetailed = default(false);
        bool hotPlug = default(false);
        
        int target_vendor_id = default(0x0);
		int target_product_id = default(0x0);
		int target_interfaceNumber = default(0);
		int target_interruptEP = default(0x80);
}


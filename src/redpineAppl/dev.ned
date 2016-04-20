
package VENTOS.src.redpineAppl;

simple dev
{
    parameters:
        @class(VENTOS::dev);
        @display("i=block/network2");
        
         int headerLength = default(88bit) @unit(bit); //header length of the application 
                
         string host = default("localhost"); 
         int port = default(22);
         string username = default("ubuntu");
         string password = default("Redpine"); 
}


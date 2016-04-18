
package VENTOS.src.codeLoader;

simple codeLoader
{
    parameters:
        @class(VENTOS::codeLoader);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        bool on = default(false);   
        string host = default("localhost"); 
        int port = default(22);
        string username = default("ubuntu");
        string password = default("Redpine");
}


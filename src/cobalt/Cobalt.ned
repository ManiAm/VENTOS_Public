
package VENTOS.src.cobalt;

simple Cobalt
{
    parameters:
        @class(VENTOS::Cobalt);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        bool active = default(false);   
        string host = default("localhost"); 
        string port = default("");
}


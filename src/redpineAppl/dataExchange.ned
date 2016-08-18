
package VENTOS.src.redpineAppl;

simple dataExchange
{
    parameters:
        @class(VENTOS::dataExchange);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        bool active = default(false);
        int VENTOS_serverPort = default(34676);
        int numBacklog = default(10);
}


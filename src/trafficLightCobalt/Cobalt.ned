
package VENTOS.src.trafficLightCobalt;

simple Cobalt
{
    parameters:
        @class(VENTOS::SNMPConnect);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        bool on = default(false);   
        string IPAddress; 
        string port;
}


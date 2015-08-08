
package c3po.ned;

simple Cobalt
{
    parameters:
        @class(VENTOS::SNMPConnect);
        @display("i=block/network2");
        
        bool on = default(false);   
        string IPAddress; 
        string port;
}



package c3po.application.trafficLightCobalt;

simple Cobalt
{
    parameters:
        @class(VENTOS::SNMPConnect);
        @display("i=block/network2");
        
        bool on = default(false);   
        string IPAddress; 
        string port;
}


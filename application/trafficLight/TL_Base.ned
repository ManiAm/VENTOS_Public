
package c3po.application.trafficLight;

simple TL_Base
{
    parameters:
        @class(VENTOS::TrafficLightBase);
        @display("i=block/network2");  
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        int TLControlMode;
}

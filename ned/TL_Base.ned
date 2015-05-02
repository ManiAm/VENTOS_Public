
package c3po.ned;

simple TL_Base
{
    parameters:
        @class(VENTOS::TrafficLightBase);
        @display("i=block/network2");  
        
        bool collectTLData;  
        int TLControlMode;     
}


package c3po.ned;

simple TL_Base
{
    parameters:
        @class(VENTOS::TrafficLightBase);
        @display("i=block/network2");  
        
        int TLControlMode; 
        bool activeDetection;  
        int debugLevel;  
}

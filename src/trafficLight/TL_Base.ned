
package VENTOS.src.trafficLight;

simple TL_Base
{
    parameters:
        @class(VENTOS::TrafficLightBase);
        @display("i=block/network2");  
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        
        // TLControlMode = 0: no traffic light
        // TLControlMode = 1: fix-time
        // TLControlMode = 2: adaptive Webster
        // TLControlMode = 3: traffic actuated
        // TLControlMode = 4: LQF (with no starvation)
        // TLControlMode = 5: OJF (lowest delay)
        // TLControlMode = 6: LQF_MWM (has starvation)
        // TLControlMode = 7: LQF_MWM_Aging
        // TLControlMode = 8: LQF_FMSC
        // TLControlMode = 9: router
        int TLControlMode = default(0);
}


package VENTOS.src.trafficLight;

simple TL_Base
{
    parameters:
        @class(VENTOS::TrafficLightBase);
        @display("i=block/network2");  
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        
        // TLControlMode = 0:  no traffic light
        // TLControlMode = 1:  fix-time
        // TLControlMode = 2:  adaptive Webster
        // TLControlMode = 3:  traffic actuated
        // TLControlMode = 4:  LQF (with no starvation)
        // TLControlMode = 5:  OJF (lowest delay)
        // TLControlMode = 6:  LQF_MWM (has starvation)
        // TLControlMode = 7:  LQF_MWM_Aging
        // TLControlMode = 8:  FMSC
        // TLControlMode = 9:  router
        // TLControlMode = 10: DRL
        int TLControlMode = default(0);
}


simple TL_IntersectionQueue extends TL_Base
{
    parameters:
        @class(VENTOS::IntersectionQueue);
        @display("i=block/network2"); 
        
        bool record_intersectionQueue_stat = default(false);
        
        double speedThreshold_veh  = default(5.0);  // m/s: vehicle with speed less than this value is considered stopped
        double speedThreshold_bike = default(0.4);  // m/s: bike with speed less than this value is considered stopped
        
        int queueSizeLimit = default(-1); // max queue has the accurate queue size information 
}


simple TL_IntersectionDemand extends TL_IntersectionQueue
{
    parameters:
        @class(VENTOS::IntersectionDemand);
        @display("i=block/network2"); 
        
        bool record_trafficDemand_stat = default(false);  
        int trafficDemandMode = default(2);        // =1: measure headway time  =2: measure total # of vehicles in a period (more accurate)
        int trafficDemandBuffSize = default(10);   // buffer size for storing traffic demands 
}


simple IntersectionDelay extends TL_IntersectionDemand
{
    parameters:
        @class(VENTOS::IntersectionDelay);
        @display("i=block/network2");  
        
        bool record_intersectionDelay_stat = default(false);    
        
        double deccelDelayThreshold = default(-1);  // m/s^2: vehicle/bike with decel less than this value is considered delayed 
}


simple TL_AllowedMoves extends IntersectionDelay
{
    parameters:
        @class(VENTOS::TrafficLightAllowedMoves);
        @display("i=block/network2");        
}


simple TL_TLStateRecord extends TL_AllowedMoves
{
    parameters:
        @class(VENTOS::TLStateRecord);
        @display("i=block/network2");  
        
        double minGreenTime = default(8.3s) @unit(s);  // long enough to clear the queue (5 vehicles)
        double maxGreenTime = default(20.0s) @unit(s); // about twice as minGreenTime --should not exceed green time in fix-time (bigger value negatively affect performance!)
        double yellowTime = default(4.0s) @unit(s);    // yellow time should be >= 3s
        double redTime = default(2.0s) @unit(s);
        double maxCycleLength = default(120s) @unit(s);  
}


simple TL_Fixed extends TL_TLStateRecord
{
    parameters:
        @class(VENTOS::TrafficLightFixed);
        @display("i=block/network2");        
}


simple TL_Webster extends TL_Fixed
{
    parameters:
        @class(VENTOS::TrafficLightWebster);
        @display("i=block/network2");    
        
        double alpha = default(0.125);          
}


simple TL_Actuated extends TL_Webster
{
    parameters:
        @class(VENTOS::TrafficLightActuated);
        @display("i=block/network2");        
        
        bool greenExtension = default(true);
        
        // -1 : passage time is calculated precisely for each incoming lane (might not be realistic!)
        // > 0: value set by the user (2.0s gives good results if LD is at 35m distance)
        double passageTime = default(-1s) @unit(s);
}


simple TL_LQF extends TL_Actuated
{
    parameters:
        @class(VENTOS::TrafficLightLQF_NoStarv);
        @display("i=block/network2");             
}


simple TL_OJF extends TL_LQF
{
    parameters:
        @class(VENTOS::TrafficLightOJF);
        @display("i=block/network2");              
}


simple TL_LQF_MWM extends TL_OJF
{
    parameters:
        @class(VENTOS::TrafficLight_LQF_MWM);
        @display("i=block/network2");          
}


simple TL_OJFM extends TL_LQF_MWM
{
    parameters:
        @class(VENTOS::TrafficLight_OJFM);
        @display("i=block/network2");        
}


simple TL_Router extends TL_OJFM
{
    parameters:
        @class(VENTOS::TrafficLightRouter);
        @display("i=block/network2");  
        
        // 0=no TLControl, 1=HighDensity, 2=LowDensity 3=Cooperative
        int TLLogicMode = default(0);
        
        double HighDensityRecalculateFrequency = default(30);
        double LowDensityExtendTime = default(3);
        double MaxPhaseDuration = default(60);  // Low density/Cooperative will not extend traffic lights to last longer than this  
        double MinPhaseDuration = default(10);  // Cooperative control will not assign a phase duration shorter than this
}


simple TL_Manager extends TL_Router
{
    parameters:
        @class(VENTOS::TrafficLightManager);
        @display("i=block/network2");    
}

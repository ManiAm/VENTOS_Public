
package c3po.application.router;

simple Router
{
    parameters:
        @class(VENTOS::Router);
        @display("i=block/network2");  
        
        bool enableRouter = default(false);
      
        double leftTurnCost;
        double rightTurnCost;
        double straightCost;
        double uTurnCost;
        double TLLookahead;
        int timePeriodMax;
        
        bool collectVehicleTimeData;
        int createTime;
        int vehicleCount;
        double ReroutingVehiclePercent;
        bool UseHysteresis;
        
        double EWMARate;
        
        int LaneCostsMode;
        int HysteresisCount;

        bool UseAccidents;
        int AccidentCheckInterval;
        
        int dijkstraOutdateTime;
}


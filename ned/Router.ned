
package c3po.ned;

simple Router
{
    parameters:
        @class(VENTOS::Router);
        @display("i=block/network2");  
        
        bool enableRouting = default(false);
      
        double leftTurnCost;
        double rightTurnCost;
        double straightCost;
        double uTurnCost;
        double TLLookahead;
        int timePeriodMax;
        
        bool collectVehicleTimeData;
        int createTime;
        int vehicleCount;
        double nonReroutingVehiclePercent;
        bool UseHysteresis;
        
        int LaneCostsMode;
        int HysteresisCount;

        bool UseAccidents;
        int AccidentCheckInterval;
}


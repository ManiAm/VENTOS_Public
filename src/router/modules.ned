
package VENTOS.src.router;

simple Router
{
    parameters:
        @class(VENTOS::Router);
        @display("i=block/network2");  
        @signal[router]();
        
        bool enableRouter = default(false);
        bool collectVehicleTimeData = default(false);
        
        double leftTurnCost = default(6.0);
        double rightTurnCost = default(4.0);
        double straightCost = default(2.0);
        double uTurnCost = default(6.0);
        
        double TLLookahead = default(0);
        int createTime = default(600);        
        int vehicleCount = default(800);    
        double ReroutingVehiclePercent = default(0.2);
        bool UseHysteresis = default(true);        
        double EWMARate = default(0.2);
        
        int LaneCostsMode = default(0);
        int HysteresisCount = default(4);
        
        bool UseAccidents = default(false);
        int AccidentCheckInterval = default(5);
        
        int dijkstraOutdateTime = default(5);
        int timePeriodMax = default(250);
}


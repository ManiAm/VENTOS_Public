
package c3po.ned;

simple Router
{
    parameters:
        @class(VENTOS::Router);
        @display("i=block/network2");  
      
        double leftTurnCost;
        double rightTurnCost;
        double straightCost;
        double uTurnCost;
        double TLLookahead;
        int timePeriod;
        
        bool enableRouting = default(false); // build router-side networks 
}


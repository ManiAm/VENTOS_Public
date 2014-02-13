
package c3po.ned;
import org.mixim.modules.mobility.traci.TraCIScenarioManagerLaunchd;

simple Warmup
{
    parameters:
        @class(Warmup);
        @display("i=block/network2");
        
        bool on = default(false);
        string leadingVehicle;
        double stopPosition;   
        double waitingTime;     
}



package c3po.ned;
import org.mixim.modules.mobility.traci.TraCIScenarioManagerLaunchd;

simple MyTraCI extends TraCIScenarioManagerLaunchd
{
    parameters:
        @class(MyTraCI);
        @display("i=block/network2");
        
        bool exTrajectory = default(false);
        string trajectory;

}


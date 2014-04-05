
package c3po.ned;
import org.mixim.modules.mobility.traci.TraCIScenarioManagerLaunchd;
// import org.car2x.veins.modules.mobility.traci.TraCIScenarioManagerLaunchd;

simple SpeedProfile
{
    parameters:
        @class(SpeedProfile);
        @display("i=block/network2");
        
        bool on = default(false);
        string laneId;
        int mode = default(1);        
        double minSpeed;
        double normalSpeed;
        double maxSpeed;
        double switchTime = default(40s) @unit(s);
}


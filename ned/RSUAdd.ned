
package c3po.ned;

import org.car2x.veins.modules.mobility.traci.TraCIScenarioManagerLaunchd;

simple RSUAdd
{
    parameters:
        @class(RSUAdd);
        @display("i=block/network2");
        
        bool on = default(true);
        int mode;  
        string RSUfile = default("");
}


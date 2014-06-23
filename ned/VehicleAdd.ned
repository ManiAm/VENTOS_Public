
package c3po.ned;

import org.car2x.veins.modules.mobility.traci.TraCIScenarioManagerLaunchd;

simple VehicleAdd
{
    parameters:
        @class(VehicleAdd);
        @display("i=block/network2");
        
        bool on = default(true);
        int mode;
        int totalVehicles;
}


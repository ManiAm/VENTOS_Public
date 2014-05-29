
package c3po.ned;

import org.mixim.base.connectionManager.ConnectionManager;
//import org.car2x.veins.base.connectionManager.ConnectionManager;

import org.mixim.base.modules.BaseWorldUtility;
//import org.car2x.veins.base.modules.BaseWorldUtility;

import org.mixim.modules.mobility.traci.TraCIScenarioManagerLaunchd;
//import org.car2x.veins.modules.mobility.traci.TraCIScenarioManagerLaunchd;

import org.mixim.modules.obstacle.ObstacleControl;
//import org.car2x.veins.modules.obstacle.ObstacleControl;

import org.mixim.modules.world.annotations.AnnotationManager;
//import org.car2x.veins.modules.world.annotations.AnnotationManager;


network Network
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        // do not put @display here!
        // it automatically adjust itself based on the playground size in ini
    submodules:
        obstacles: ObstacleControl {
            @display("p=274,283");
        }
        annotations: AnnotationManager {
            @display("p=369,283");
        }
        connectionManager: ConnectionManager {
            parameters:
                @display("p=153,283;i=abstract/multicast");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=50,283;i=misc/globe");
        }
        manager: TraCI_App {
            parameters:
                @display("p=153,372");
        }
        vehicleAdd: VehicleAdd {
            parameters:
                @display("p=235,372");
        }
        warmup: Warmup {
            parameters:
                @display("p=323,372");
        }
        speedprofile: SpeedProfile {
            parameters:
                @display("p=414,372");
        }
        statistics: Statistics {
            parameters:
                @display("p=61,372;i=block/timer");
        }
        trafficLight: TrafficLight {
            parameters:
                @display("p=61,471;i=block/broadcast");
        }

        adversary: Adversary {
            parameters:
                @display("p=155,475;i=old/comp_a");
        }
    connections allowunconnected:
}


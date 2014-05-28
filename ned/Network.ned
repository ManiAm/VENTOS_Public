
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
        @display("bgb=540,319");
    submodules:
        obstacles: ObstacleControl {
            @display("p=262,58");
        }
        annotations: AnnotationManager {
            @display("p=357,58");
        }
        connectionManager: ConnectionManager {
            parameters:
                @display("p=141,58;i=abstract/multicast");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=38,58;i=misc/globe");
        }
        manager: TraCI_App {
            parameters:
                @display("p=141,147");
        }
        vehicleAdd: VehicleAdd {
            parameters:
                @display("p=223,147");
        }
        warmup: Warmup {
            parameters:
                @display("p=311,147");
        }
        speedprofile: SpeedProfile {
            parameters:
                @display("p=402,147");
        }
        statistics: Statistics {
            parameters:
                @display("p=49,147;i=block/timer");
        }
        trafficLight: TrafficLight {
            parameters:
                @display("p=49,246;i=block/broadcast");
        }

        adversary: Adversary {
            parameters:
                @display("p=143,250;i=old/comp_a");
        }
    connections allowunconnected:
}


package c3po.ned;

import org.car2x.veins.base.connectionManager.ConnectionManager;
import org.car2x.veins.base.modules.BaseWorldUtility;
import org.car2x.veins.modules.mobility.traci.TraCIScenarioManagerLaunchd;
import org.car2x.veins.modules.obstacle.ObstacleControl;
import org.car2x.veins.modules.world.annotations.AnnotationManager;

network Network
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)

        string SUMODirectory = default(""); // location of SUMO .ned and .rou files

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
        TraCI: TraCI_App {
            parameters:
                @display("p=145,372");
        }
        Tracking: Tracking {
            parameters:
                @display("p=221,372");
        }
        vehicleAdd: VehicleAdd {
            parameters:
                @display("p=301,372");
        }
        warmup: Warmup {
            parameters:
                @display("p=61,459");
        }
        speedprofile: SpeedProfile {
            parameters:
                @display("p=145,459");
        }
        statistics: Statistics {
            parameters:
                @display("p=61,372;i=block/timer");
        }
        router: Router {
            parameters:
                @display("p=235,459;i=block/dispatch");
        }
        TrafficLight: TrafficLight {
            parameters:
                @display("p=320,459;i=block/dispatch");
        }
        RSUAdd: RSUAdd {
            parameters:
                @display("p=381,372");
        }
        plotter: Plotter {
            parameters:
                @display("p=61,548;i=block/app");
        }

    connections allowunconnected:
}


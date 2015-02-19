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
        string trafficLightType = default("");

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
                @display("p=140,372");
        }
        SumoBinary: SumoBinary {
            parameters:
                @display("p=222,372");
        }
        Tracking: Tracking {
            parameters:
                @display("p=303,372");
        }
        addVehicle: AddVehicle {
            parameters:
                @display("p=61,459;i=old/bwsink");
        }
        addRSU: AddRSU {
            parameters:
                @display("p=176,459;i=old/bwsink");
        }
        addAdversary: AddAdversary {
            parameters:
                @display("p=280,459;i=old/bwsink");
        }
        warmup: Warmup {
            parameters:
                @display("p=61,546");
        }
        speedprofile: SpeedProfile {
            parameters:
                @display("p=176,546");
        }
        statistics: Statistics {
            parameters:
                @display("p=61,372;i=block/timer");
        }
        router: Router {
            parameters:
                @display("p=61,639;i=block/dispatch");
        }
        TrafficLight: <trafficLightType> like TrafficLightInterface {
            parameters:
                @display("p=176,639;i=block/dispatch");
        }
        plotter: Plotter {
            parameters:
                @display("p=369,372;i=block/app");
        }

    connections allowunconnected:
}


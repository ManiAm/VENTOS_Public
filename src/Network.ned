package VENTOS.src;

import VENTOS.src.MIXIM.modules.*;
import VENTOS.src.MIXIM.connectionManager.*;
import VENTOS.src.traci.*;
import VENTOS.src.global.*;
import VENTOS.src.addNode.*;
import VENTOS.src.router.*;
import VENTOS.src.sniffing.*;
import VENTOS.src.trafficLight.*;
import VENTOS.src.trafficLightCobalt.*;
import VENTOS.src.veins.*;

network Network
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)

        int debugLevel;

        @display("bgb=389,470");
    submodules:
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=48,43;i=misc/globe");
        }
        connMan: ConnectionManager {
            parameters:
                @display("p=151,43;i=abstract/multicast");
        }
        TraCI: TraCIStart {
            parameters:
                @display("p=48,135");
        }
        Tracking: Tracking {
            parameters:
                @display("p=245,212;i=block/circle");
        }
        addScenario: AddScenario {
            parameters:
                @display("p=245,135;i=old/bwsink");
        }
        warmup: Warmup {
            parameters:
                @display("p=48,212;i=block/table");
        }
        speedprofile: SpeedProfile {
            parameters:
                @display("p=144,212;i=block/table");
        }
        statistics: Statistics {
            parameters:
                @display("p=144,135;i=block/timer");
        }
        router: Router {
            parameters:
                @display("p=245,389;i=block/broadcast");
        }
        TrafficLight: TL_Manager {
            parameters:
                @display("p=48,389;i=block/broadcast");
        }
        cobalt: Cobalt {
            parameters:
                @display("p=144,389;i=block/broadcast");
        }
        plotter: Plotter {
            parameters:
                @display("p=338,212;i=block/app");
        }
        sniffEthernet: SniffEthernet {
            parameters:
                @display("p=48,299;i=block/network");
        }
        sniffUSB: SniffUSB {
            parameters:
                @display("p=144,299;i=block/mac");
        }
        sniffBluetooth: SniffBluetoothDump {
            parameters:
                @display("p=245,299;i=block/mac");
        }

    connections allowunconnected:
}


package c3po.application;

import c3po.application.MIXIM.modules.*;
import c3po.application.MIXIM.connectionManager.*;
import c3po.application.traci.*;
import c3po.application.global.*;
import c3po.application.router.*;
import c3po.application.sniffing.*;
import c3po.application.trafficLight.*;
import c3po.application.trafficLightCobalt.*;
import c3po.application.veins.*;

network Network
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)

        string SUMODirectory = default(""); // location of SUMO files
        int debugLevel;

        @display("bgb=382,627");
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
        TraCI: TraCI_Start {
            parameters:
                @display("p=48,132");
        }
        SumoServer: SumoServer {
            parameters:
                @display("p=151,132");
        }
        Tracking: Tracking {
            parameters:
                @display("p=209,381;i=block/circle");
        }
        addEntity: AddEntity {
            parameters:
                @display("p=48,296;i=old/bwsink");
        }
        addFlow: AddFlow {
            parameters:
                @display("p=127,296;i=old/bwsink");
        }
        addRSU: AddRSU {
            parameters:
                @display("p=209,296;i=old/bwsink");
        }
        addAdversary: AddAdversary {
            parameters:
                @display("p=297,296;i=old/bwsink");
        }
        warmup: Warmup {
            parameters:
                @display("p=48,381;i=block/table");
        }
        speedprofile: SpeedProfile {
            parameters:
                @display("p=127,381;i=block/table");
        }
        statistics: Statistics {
            parameters:
                @display("p=48,216;i=block/timer");
        }
        router: Router {
            parameters:
                @display("p=202,558;i=block/broadcast");
        }
        TrafficLight: TL_Manager {
            parameters:
                @display("p=48,558;i=block/broadcast");
        }
        cobalt: Cobalt {
            parameters:
                @display("p=130,558;i=block/broadcast");
        }
        plotter: Plotter {
            parameters:
                @display("p=294,381;i=block/app");
        }
        sniffEthernet: SniffEthernet {
            parameters:
                @display("p=48,468;i=block/network");
        }
        sniffUSB: SniffUSB {
            parameters:
                @display("p=130,468;i=block/mac");
        }
        sniffBluetooth: SniffBluetoothDump {
            parameters:
                @display("p=221,468;i=block/mac");
        }

    connections allowunconnected:
}


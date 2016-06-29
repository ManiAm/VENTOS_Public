package VENTOS.src;

import VENTOS.src.MIXIM.modules.*;
import VENTOS.src.MIXIM.connectionManager.*;
import VENTOS.src.traci.*;
import VENTOS.src.global.*;
import VENTOS.src.logging.*;
import VENTOS.src.addNode.*;
import VENTOS.src.router.*;
import VENTOS.src.interfacing.*;
import VENTOS.src.redpineAppl.*;
import VENTOS.src.trafficLight.*;
import VENTOS.src.cobalt.*;
import VENTOS.src.veins.*;

network Network
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)

        // 0=no debug. 
        // Higher numbers output more debug info
        // debug info is only available in OMNET GUI mode
        int debugLevel = default(0);

        @display("bgb=489,453");
    submodules:
        vLogging: vlog {
            parameters:
                @display("p=48,48;i=old/floppy2");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=144,47;i=misc/globe");
        }
        connMan: ConnectionManager {
            parameters:
                @display("p=245,48;i=abstract/multicast");
        }
        TraCI: TraCIStart {
            parameters:
                @display("p=144,135");
        }
        Tracking: Tracking {
            parameters:
                @display("p=48,135;i=block/circle");
        }
        addFixNode: AddFixNode {
            parameters:
                @display("p=245,135;i=old/bwsink");
        }
        addMobileNode: AddMobileNode {
            parameters:
                @display("p=337,135;i=old/bwsink");
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
                @display("p=337,47;i=block/timer");
        }
        router: Router {
            parameters:
                @display("p=245,212;i=block/broadcast");
        }
        TrafficLight: TL_Manager {
            parameters:
                @display("p=337,212;i=block/broadcast");
        }
        cobalt: Cobalt {
            parameters:
                @display("p=432,212;i=block/broadcast");
        }
        plotter: Plotter {
            parameters:
                @display("p=432,47;i=block/app");
        }
        Ethernet: Ethernet {
            parameters:
                @display("p=48,299;i=block/network");
        }
        USB: USB {
            parameters:
                @display("p=144,299;i=block/mac");
        }
        Bluetooth: BLE_Dump {
            parameters:
                @display("p=245,299;i=device/wifilaptop");
        }
        dataExchange: dataExchange {
            parameters:
                @display("p=144,391;i=block/ifcard");
        }
        codeLoader: codeLoader {
            parameters:
                @display("p=48,391;i=block/ifcard");
        }

    connections allowunconnected:
}


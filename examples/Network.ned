package VENTOS.examples;

import VENTOS.src.traci.*;
import VENTOS.src.global.*;
import VENTOS.src.gettingStarted.*;
import VENTOS.src.logging.*;
import VENTOS.src.addNode.*;
import VENTOS.src.router.*;
import VENTOS.src.trafficLight.*;
import VENTOS.src.MIXIM_veins.connectionManager.*;

network Network
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)

        string traciType;
        string statisticsType;
        string addNodeType;

        // 0=no debug. 
        // Higher numbers output more debug info
        // debug info is only available in OMNET GUI mode
        int debugLevel = default(0);

        @display("bgb=529,283");
    submodules:
        // TraCI is the first module so that its finish method
        // is called before all other modules to record simulation end time
        TraCI: <traciType> like IBaseTraCI {
            parameters:
                @display("p=48,135");
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
        vLogging: vglog {
            parameters:
                @display("p=48,48;i=old/floppy2");
        }
        gui: gui {
            parameters:
                @display("p=337,135;i=block/circle");
        }
        addNode: <addNodeType> like IBaseAddNode {
            parameters:
                @display("p=144,135;i=old/bwsink");
        }
        trafficControl: TrafficControl {
            parameters:
                @display("p=245,135;i=block/app2");
        }
        statistics: <statisticsType> like IBaseStatistics {
            parameters:
                @display("p=337,47;i=block/timer");
        }
        inductionLoop: InductionLoop {
            parameters:
                @display("p=436,47;i=block/timer");
        }
        router: Router {
            parameters:
                @display("p=48,213;i=block/broadcast");
        }
        TrafficLight: TL_Manager {
            parameters:
                @display("p=144,213;i=block/broadcast");
        }
        tutorial: tutorial {
            parameters:
                @display("p=245,216;i=old/ring");
        }

    connections allowunconnected:
}


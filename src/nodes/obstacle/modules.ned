
package VENTOS.src.nodes.obstacle;

import VENTOS.src.MIXIM_veins.nic.Nic80211p;
import VENTOS.src.mobility.BaseMobility;

module obstacle
{
    parameters:
        @signal[beaconSignaling]();
        
        bool record_stat = default(false);
        string record_list = default("vehId | lane | lanePos | speed");
        
        string SUMOID = default("");
        string SUMOType = default("");
        string vehicleClass = default("");
        
        bool DSRCenabled = default(false);
        bool hasOBU = default(false);
        string IPaddress = default("");
        
    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        appl: ApplObstacleManager {
            parameters:
                @display("p=60,50");
        }

        nic: Nic80211p {
            parameters:
                @display("p=60,166");
        }

        mobility: BaseMobility {
            parameters:
                @display("p=130,172;i=block/cogwheel");
        }
    connections:
        nic.upperLayerOut --> appl.lowerLayerIn;
        nic.upperLayerIn <-- appl.lowerLayerOut;
        nic.upperControlOut --> appl.lowerControlIn;
        nic.upperControlIn <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;
}


import VENTOS.src.baseAppl.BaseWaveApplLayer;

simple ApplObstacleBeacon extends BaseWaveApplLayer
{
    parameters:
        @class(VENTOS::ApplObstacleBeacon);
        @display("i=block/app2");
}


simple ApplObstacleManager extends ApplObstacleBeacon
{
    parameters:
        @class(VENTOS::ApplObstacleManager);
        @display("i=block/app2");
}


package VENTOS.src.nodes.pedestrian;

import VENTOS.src.MIXIM_veins.nic.Nic80211p;
import VENTOS.src.mobility.TraCIMobilityMod;

module pedestrian
{
    parameters:
        @signal[beaconSignaling]();
        
        bool record_stat = default(false);
        string record_list = default("timeStep | id | lane | lanePos | speed");
        
        string SUMOID = default("");
        string SUMOType = default("");
        string vehicleClass = default("");
        
        bool DSRCenabled = default(true);
        bool hasOBU = default(false);
        string IPaddress = default("");
        
    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        appl: ApplPedManager {    // <applType> like IBaseApplLayer {
            parameters:
                @display("p=60,50");
        }
        
        nic: Nic80211p {
            parameters:
                @display("p=60,166");
        }
        
        mobility: TraCIMobilityMod {
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

simple ApplPedBeacon extends BaseWaveApplLayer
{
    parameters:
        @class(VENTOS::ApplPedBeacon);
        @display("i=block/app2");
                
        bool signalBeaconing = default(false); // vehicles send beacons using signals rather than wireless broadcast. Any module
                                              // registered to signalBeaconing signal will receive the beacon 
}


simple ApplPedManager extends ApplPedBeacon
{
    parameters:
        @class(VENTOS::ApplPedManager);
        @display("i=block/app2");
}


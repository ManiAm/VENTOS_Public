
package VENTOS.src.nodes.bicycle;

import VENTOS.src.MIXIM_veins.nic.Nic80211p;
import VENTOS.src.mobility.TraCIMobility_Mod;

module bicycle
{
    parameters:
        @signal[beaconSignaling]();
        
        bool record_stat = default(false);
        string record_list = default("vehId | lane | lanePos | speed");
        
        string SUMOID = default("");
        string SUMOType = default("");
        string vehicleClass = default("");
        
        bool DSRCenabled = default(true);
        bool hasOBU = default(false);
        string IPaddress = default("");
        
    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        appl: ApplBikeManager {    // <applType> like IBaseApplLayer {
            parameters:
                @display("p=60,50");
        }

        nic: Nic80211p {
            parameters:
                @display("p=60,166");
        }

        mobility: TraCIMobility_Mod {
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

simple ApplBikeBeacon extends BaseWaveApplLayer
{
    parameters:
        @class(VENTOS::ApplBikeBeacon);
        @display("i=block/app2");
        
        bool signalBeaconing = default(false); // vehicles send beacons using signals rather than wireless broadcast. Any module
                                               // registered to signalBeaconing signal will receive the beacon 
}


simple ApplBikeManager extends ApplBikeBeacon
{
    parameters:
        @class(VENTOS::ApplBikeManager);
        @display("i=block/app2");
}

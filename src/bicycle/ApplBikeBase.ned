
package VENTOS.src.bicycle;

import VENTOS.src.MIXIM.modules.IBaseApplLayer;

simple ApplBikeBase like IBaseApplLayer
{
    parameters:
        @class(VENTOS::ApplBikeBase);
        
        int headerLength = default(256bit) @unit(bit); //header length of the application        
        bool debug = default(false); //whether to print debug output
        
        string SUMOID = default("");
        string SUMOType = default("");
        string vehicleClass = default("");
        int vehicleClassEnum = default(-1);
        
        // bike does not need these two varibales but
        // it makes the job simpler for TraCIstart.cc
        int SUMOControllerType = default(-1);
        int SUMOControllerNumber = default(-1);
        
        bool hasOBU = default(false);
        string IPaddress = default("");

    gates:
        input lowerLayerIn;    // from mac layer
        output lowerLayerOut;  // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}

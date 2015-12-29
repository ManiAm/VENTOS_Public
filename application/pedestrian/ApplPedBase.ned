
package c3po.application.pedestrian;
import c3po.application.MIXIM.modules.*;

simple ApplPedBase like IBaseApplLayer
{
    parameters:
        @class(VENTOS::ApplPedBase);
        
        int headerLength = default(88bit) @unit(bit); //header length of the application        
        bool debug = default(false); //whether to print debug output
        
        string SUMOID = default("");
        string SUMOType = default("");
        string vehicleClass = default("");
        int vehicleClassEnum = default(-1);
        int SUMOControllerType = default(-1);
        int SUMOControllerNumber = default(-1);

    gates:
        input lowerLayerIn;    // from mac layer
        output lowerLayerOut;  // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}

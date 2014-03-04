package c3po.ned;
import org.mixim.base.modules.IBaseApplLayer;
//import org.car2x.veins.base.modules.IBaseApplLayer;


simple ApplVBase like IBaseApplLayer
{
    parameters:
        @class(ApplVBase);
        
        int headerLength = default(88bit) @unit(bit); //header length of the application        
        bool debug = default(false); //whether to print debug output

    gates:
        input lowerLayerIn;    // from mac layer
        output lowerLayerOut;  // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}

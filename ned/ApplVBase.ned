package c3po.ned;
import org.mixim.base.modules.IBaseApplLayer;

simple ApplVBase like IBaseApplLayer
{
    parameters:
        @class(ApplVBase);
        
        int headerLength = default(88bit) @unit(bit); //header length of the application        
        bool debug = default(false); //whether to print debug output
               
        bool sendData = default(false); //whether this applayer should send data
        int dataLengthBits = default(1024bit) @unit(bit); //the length of a data packet
        bool dataOnSch = default(true); //tells the applayer whether to use a service channel for datapackets or the control channel
        int dataPriority = default(3); //the access category for data packets

    gates:
        input lowerLayerIn;    // from mac layer
        output lowerLayerOut;  // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}

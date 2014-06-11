package c3po.ned;
import org.mixim.base.modules.IBaseApplLayer;
//import org.car2x.veins.base.modules.IBaseApplLayer;


simple ApplAdversary like IBaseApplLayer
{
    parameters:
        @class(ApplAdversary);
        
        int headerLength = default(88bit) @unit(bit); // header length of the application   
        bool on = default(false); 
        double AttackT;
        bool falsificationAttack = default(false);  
        bool replayAttack = default(false); 
        bool jammingAttack = default(false);  

    gates:
        input lowerLayerIn;    // from mac layer
        output lowerLayerOut;  // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}

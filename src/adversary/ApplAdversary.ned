
package VENTOS.src.adversary;

import VENTOS.src.MIXIM.modules.IBaseApplLayer;

simple ApplAdversary like IBaseApplLayer
{
    parameters:
        @class(VENTOS::ApplAdversary);
        
        bool on = default(false); 
        int headerLength = default(256bit) @unit(bit); // header length of the application 
          
        double AttackT = default(100);
        bool falsificationAttack = default(false);  
        bool replayAttack = default(false); 
        bool jammingAttack = default(false);  

    gates:
        input lowerLayerIn;    // from mac layer
        output lowerLayerOut;  // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}

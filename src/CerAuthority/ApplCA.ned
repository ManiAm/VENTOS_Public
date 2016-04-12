
package VENTOS.src.CerAuthority;

import VENTOS.src.MIXIM.modules.IBaseApplLayer;

simple ApplCA like IBaseApplLayer
{
    parameters:
        @class(VENTOS::ApplCA);
        
        bool active = default(false); 
        int headerLength = default(256bit) @unit(bit); // header length of the application    
        
        int InitialWait = default(0);
        int CRLsize = default(1000);
        bool EnableShuffle = default(false);
        bool ErasureCode = default(true);
        
        int NoSegments = default(20);  // number of CRL pieces (only when ErasureCode = false)

        int M = default(20);
        int N = default(30); 
        
        double Pseudonym_lifeTime = default(120s) @unit(s);

    gates:
        input lowerLayerIn;    // from mac layer
        output lowerLayerOut;  // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}

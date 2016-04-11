
package VENTOS.src.CerAuthority;

import VENTOS.src.MIXIM.modules.IBaseApplLayer;

simple ApplCA like IBaseApplLayer
{
    parameters:
        @class(VENTOS::ApplCA);
        
        int headerLength = default(88bit) @unit(bit); // header length of the application   
        bool active = default(false);  
        
        int InitialWait;
        int CRLsize;
        bool EnableShuffle;
        bool ErasureCode;
        
        int NoSegments;        

        int M;
        int N; 
        
        double Pseudonym_lifeTime @unit(s);

    gates:
        input lowerLayerIn;    // from mac layer
        output lowerLayerOut;  // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}


package VENTOS.src.nodeInsertion;

simple AddFixNode
{
    parameters:
        @class(VENTOS::AddFixNode);
        @display("i=block/network2");
        
        int headerLength = default(256bit) @unit(bit); //header length of the application   
        
        int numRSUs = default(0);
        string moduleRSU = default("VENTOS.src.rsu.RSU");
        
        int numObstacles = default(0);
        string moduleObstacle = default("VENTOS.src.vehicle.vehicle");
        
        int numAdversary = default(0);
        string moduleAdversary = default("VENTOS.src.adversary.Adversary");
        
        int numCA = default(0);
        string moduleCA = default("VENTOS.src.CerAuthority.CA");
}

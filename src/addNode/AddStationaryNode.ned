
package VENTOS.src.addNode;

simple AddStationaryNode
{
    parameters:
        @class(VENTOS::AddStationaryNode);
        @display("i=block/network2");
        
        int headerLength = default(256bit) @unit(bit); //header length of the application   
        
        int numRSUs = default(0);
        int numAdversary = default(0);
        int numCA = default(0);
}


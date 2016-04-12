
package VENTOS.src.addNode;

simple AddNode
{
    parameters:
        @class(VENTOS::AddNode);
        @display("i=block/network2"); 
        
        string RSUfile = default("RSUsLocation.xml");
        string flowFile = default("flows.xml");
        string flowSetId;
}


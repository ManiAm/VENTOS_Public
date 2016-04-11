
package VENTOS.src.MIXIM.modules;

// Interface for mobility modules
moduleinterface IMobility
{
    parameters:
        @display("i=block/cogwheel");
                
        double x; // x coordinate of the nodes' position (-1 = random)
        double y; // y coordinate of the nodes' position (-1 = random)
        double z; // z coordinate of the nodes' position (-1 = random)
}



package VENTOS.src.nodeInsertion;

simple Warmup
{
    parameters:
        @class(VENTOS::Warmup);
        @display("i=block/network2");
        
        bool active = default(false);
        
        string laneId = default("1to2_0");
        double stopPosition = default(9); 
        double warmUpSpeed = default(0);  
        double waitingTime = default(15);  // waiting time after all vehicles enter the simulation
}


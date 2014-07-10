
package c3po.ned;


simple Warmup
{
    parameters:
        @class(Warmup);
        @display("i=block/network2");
        
        bool on = default(false);
        string laneId;
        double stopPosition;   
        double waitingTime;     
}


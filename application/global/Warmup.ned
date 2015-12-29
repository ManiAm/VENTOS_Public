
package c3po.application.global;

simple Warmup
{
    parameters:
        @class(VENTOS::Warmup);
        @display("i=block/network2");
        
        bool on = default(false);
        string laneId;
        double stopPosition; 
        double warmUpSpeed;  
        double waitingTime;     
}


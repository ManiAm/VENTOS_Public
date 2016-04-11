
package VENTOS.src.global.addToSim;

simple AddAdversary
{
    parameters:
        @class(VENTOS::AddAdversary);
        @display("i=block/network2");
        
        bool on = default(false);
        int mode;  
}


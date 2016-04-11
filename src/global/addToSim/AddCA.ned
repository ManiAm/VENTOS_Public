
package VENTOS.src.global.addToSim;

simple AddCA
{
    parameters:
        @class(VENTOS::AddCA);
        @display("i=block/network2");
        
        bool on = default(false);
        int mode;  
}



package VENTOS.src.redpineAppl;

simple codeLoader
{
    parameters:
        @class(VENTOS::codeLoader);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        bool on = default(false);   
        string initScriptName = default("VENTOS_Start_WAVE");
        int numDev = default(0);
}


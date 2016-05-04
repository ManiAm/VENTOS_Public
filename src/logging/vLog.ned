
package VENTOS.src.logging;

simple vLog
{
    parameters:
        @class(VENTOS::vLog);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        int systemLogLevel = default(255);
        bool logRecordCMD = default(false);
}


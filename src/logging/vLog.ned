
package VENTOS.src.logging;

simple vlog
{
    parameters:
        @class(VENTOS::vlog);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        int systemLogLevel = default(255);
        bool logRecordCMD = default(false);  // show logs in cmd_env?
}



package VENTOS.src.logging;

simple vlog
{
    parameters:
        @class(VENTOS::vlog);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); //header length of the application 
        
        // relative path to mainWindow
        string loggingWindowPath = default("src/loggingWindow/mainWindow");
        int systemLogLevel = default(255);
        bool logRecordCMD = default(false);  // show logs in cmd_env?
}


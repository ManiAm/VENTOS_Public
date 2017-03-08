
package VENTOS.src.logging;

simple vlog
{
    parameters:
        @class(VENTOS::vlog);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); 
        
        bool printLogLevel = default(false);
        bool printTimeStep = default(false);
        bool printFileName = default(false);
        bool printLineNumber = default(false);
        
        bool saveLog2File = default(false);
        bool logRecordCMD = default(false);
        int systemLogLevel = default(15);
}


simple vglog extends vlog
{
    parameters:
        @class(VENTOS::vglog);
        @display("i=block/network2");
        
        // relative path to mainWindow
        string loggingWindowPath = default("src/loggingWindow/mainWindow");        
}

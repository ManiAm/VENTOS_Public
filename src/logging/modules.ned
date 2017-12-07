
package VENTOS.src.logging;

simple vlog
{
    parameters:
        @class(VENTOS::vlog);
        @display("i=block/network2");
        
        int headerLength = default(88bit) @unit(bit); 
        
        bool printLogLevel = default(false);    // print the log level (DEBUG, WARNING, etc.) along with the log message
        bool printTimeStep = default(false);    // print time step along with the log message
        bool printFileName = default(false);    // print file name along with log message
        bool printLineNumber = default(false);  // print line number along with log message
        
        bool saveLog2File = default(false);  // save a copy of log messages into a file
        bool logRecordCMD = default(false);  // show log messages in command line mode?
        
        // control which log levels should be printed?
        // 0 means no log levels, 15 means all log levels
        //           X X X X
        // WARNING:        1
        // ERROR:        1
        // INFO:       1
        // DEBUG:    1
        int systemLogLevel = default(15);        
}


simple vglog extends vlog
{
    parameters:
        @class(VENTOS::vglog);
        @display("i=block/network2");
        
        // relative path to mainWindow application
        string loggingWindowPath = default("../../src/loggingWindow/mainWindow");  
        string loggingWindowTitle = default("Log window");  // windows title
        
        bool syntaxHighlighting = default(true);
        string syntaxHighlightingExpression = default("");        
        
        bool glogRecordCMD = default(false);  // show vglog in command line mode? 
        
        bool glogActive = default(true);     
}

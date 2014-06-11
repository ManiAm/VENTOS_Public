
package c3po.ned;

simple TraCI_App extends TraCI_Extend
{
    parameters:
        @class(TraCI_App);
        @display("i=block/network2");
        
        double terminate = default(200s) @unit(s);
        
        bool tracking;
        string trackingV;
        double trackingInterval;
        
        bool adversary = default(false);        
        bool RSU = default(false);    
        string RSUfile = default(""); 
}


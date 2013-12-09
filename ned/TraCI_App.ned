
package c3po.ned;

simple TraCI_App extends TraCI_Extend
{
    parameters:
        @class(TraCI_App);
        @display("i=block/network2");
        
        double warmUpX = default(200m) @unit(m);
        double warmUpT = default(100s) @unit(s);
        
        int trajectoryMode = default(1);
        string trajectory;
        double terminate = default(200s) @unit(s);
}



package c3po.ned;

simple TraCI_App extends TraCI_Extend
{
    parameters:
        @class(TraCI_App);
        @display("i=block/network2");
        
        int trajectoryMode = default(1);
        string trajectory;
        double terminate = default(200s) @unit(s);
}


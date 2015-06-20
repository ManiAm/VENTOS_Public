
package c3po.ned;

simple TraCI_App extends TraCI_Extend
{
    parameters:
        @class(VENTOS::TraCI_App);
        @display("i=block/network2");
        
        double terminate = default(200s) @unit(s);
}


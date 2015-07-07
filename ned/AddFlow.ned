
package c3po.ned;

simple AddFlow
{
    parameters:
        @class(VENTOS::AddFlow);
        @display("i=block/network2");
        
        bool on = default(false);
        string flowFile = default("");
        int flowSet;
}


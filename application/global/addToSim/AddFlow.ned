
package c3po.application.global.addToSim;

simple AddFlow
{
    parameters:
        @class(VENTOS::AddFlow);
        @display("i=block/network2");
        
        bool on = default(false);
        string flowFile = default("");
        string flowSetId;
}


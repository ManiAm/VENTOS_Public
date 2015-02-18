
package c3po.ned;

import org.car2x.veins.modules.mobility.traci.TraCIMobility;

simple TraCIMobilityMod extends TraCIMobility
{
    parameters:
        @class(VENTOS::TraCIMobilityMod);
        @display("i=block/cogwheel");

}


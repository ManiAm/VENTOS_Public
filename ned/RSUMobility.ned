
package c3po.ned;

import org.car2x.veins.base.modules.BaseMobility;

simple RSUMobility extends BaseMobility
{
    parameters:
        @class(VENTOS::RSUMobility);
        bool debug = default(false); // debug switch
}

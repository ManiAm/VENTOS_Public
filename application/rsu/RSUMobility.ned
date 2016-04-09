
package c3po.application.rsu;

import c3po.application.MIXIM.modules.BaseMobility;

simple RSUMobility extends BaseMobility
{
    parameters:
        @class(VENTOS::RSUMobility);
        bool debug = default(false); // debug switch
}


package VENTOS.src.rsu;

import VENTOS.src.MIXIM.modules.BaseMobility;

simple RSUMobility extends BaseMobility
{
    parameters:
        @class(VENTOS::RSUMobility);
        bool debug = default(false); // debug switch
}

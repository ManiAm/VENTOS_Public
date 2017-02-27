
package VENTOS.src.mobility;

// Interface for mobility modules
moduleinterface IMobility
{
    parameters:
        @display("i=block/cogwheel");
        
        double x;  // x coordinate of the nodes' position (-1 = random)
        double y;  // y coordinate of the nodes' position (-1 = random)
        double z;  // z coordinate of the nodes' position (-1 = random)
}


// Module which is responsible for mobility
// related information like position and movement
// BaseMobility itself defines a static mobility
// pattern (means only a position, no movement).
// Mobility modules which extend from this module
// will define more complex movement patterns.
simple BaseMobility like IMobility
{
    parameters:
        @display("i=block/cogwheel");
        @signal[veinsmobilityStateChanged](type="BaseMobility");
        
        bool notAffectedByHostState = default(true);
        bool coreDebug = default(false); // debug switch for the core framework
        
        double x = default(0); // x coordinate of the nodes' position (-1 = random)
        double y = default(0); // y coordinate of the nodes' position (-1 = random)
        double z = default(0); // z coordinate of the nodes' position (-1 = random)
}


simple ConstSpeedMobility extends BaseMobility
{
    parameters:
        @class(VENTOS::ConstSpeedMobility);
        
        bool debug = default(false); // debug switch
        double speed @unit(mps) = default(0mps); // speed of the host (in m/s)
        double updateInterval @unit(s) = default(1s); // time interval to update the hosts position (seconds)
}


simple TraCIMobility_Mod extends BaseMobility
{
    parameters:
        @class(VENTOS::TraCIMobilityMod);
        @display("i=block/cogwheel");
        
        bool debug = default(false);  // debug switch
        double antennaPositionOffset @unit("m") = default(0.0m);  // position offset of the antenna of the front of the car
}


// This is a linear mobility model with speed, angle and acceleration parameters.
// Angle only changes when the mobile node hits a wall: then it reflects
// off the wall at the same angle.
//
// NOTE: Does not yet support 3-dimensional movement.
// @author Emin Ilker Cetinbas
simple LinearMobility extends BaseMobility
{
    parameters:
        @class(LinearMobility);
        
        bool debug = default(false); // debug switch
        double speed @unit(mps); // speed of the host (in m/s)
        double angle @unit(deg); // angle of linear motion (degreees)
        double acceleration @unit(mpss); // acceleration of linear motion (m/s2)
        double updateInterval @unit(s); // time interval to update the hosts position (seconds)
}


package c3po.ned;


simple SpeedProfile
{
    parameters:
        @class(VENTOS::SpeedProfile);
        @display("i=block/network2");
        
        bool on = default(false);
        string laneId;
        int mode = default(1);        
        double minSpeed;
        double normalSpeed;
        double maxSpeed;
        double switchTime = default(40s) @unit(s);
}


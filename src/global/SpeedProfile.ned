
package VENTOS.src.global;

simple SpeedProfile
{
    parameters:
        @class(VENTOS::SpeedProfile);
        @display("i=block/network2");
        
        bool active = default(false);
                
        // mode = 0: fixed speed
        // mode = 1: accel/decel pulse
        // mode = 2: accel/decel pulse (extreme)
        // mode = 3: accel/decel pulse (stability test)
        // mode = 4: accel/decel zik-zak 
        // mode = 5: accel/decel periodic
        // mode = 6: real stop-and-go traffic flow data (external file)
        int mode = default(3);
        
        string laneId = default("1to2_0");      
        double startTime = default(-1s) @unit(s);
                
        double minSpeed = default(5);
        double normalSpeed = default(20);
        double maxSpeed = default(30);
        double switchTime = default(40s) @unit(s);  // waiting time between speed change
        string trajectoryPath = default("sumo/trajectory_StopAndGo.txt");
}


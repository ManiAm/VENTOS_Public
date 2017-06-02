package VENTOS.src.addNode;

simple AddNode
{
    parameters:
        @class(VENTOS::AddNode);
        @display("i=block/network2");
        
        int headerLength = default(256bit) @unit(bit); //header length of the application 
        string id = default("");      
        
        string adversary_ModuleType = default("VENTOS.src.nodes.adversary.Adversary");
        string adversary_ModuleName = default("ADV");
        string adversary_ModuleDisplayString = default("i=status/red");        
        
        string CA_ModuleType = default("VENTOS.src.nodes.CA.CA");        
        string CA_ModuleName = default("CA");
        string CA_ModuleDisplayString = default("i=old/comp_a");   
        
        string RSU_ModuleType = default("VENTOS.src.nodes.rsu.RSU");
        string RSU_ModuleName = default("RSU");
        string RSU_ModuleDisplayString = default("i=device/antennatower");       
        
        string obstacle_ModuleType = default("VENTOS.src.nodes.obstacle.obstacle");
        string obstacle_ModuleName = default("OBS"); 
        string obstacle_ModuleDisplayString = default("i=status/error");           
        
        string vehicle_ModuleType = default("VENTOS.src.nodes.vehicle.vehicle");
        string vehicle_ModuleName = default("V"); 
        string vehicle_ModuleDisplayString = default("b=3,3,rect,red,black,1");  
        
        string bike_ModuleType = default("VENTOS.src.nodes.bicycle.bicycle");       
        string bike_ModuleName = default("B");        
        string bike_ModuleDisplayString = default("b=3,3,rect,yellow,black,1");
        
        string ped_ModuleType = default("VENTOS.src.nodes.pedestrian.pedestrian");      
        string ped_ModuleName = default("P");        
        string ped_ModuleDisplayString = default("i=abstract/person");       
}


simple AddMobileNode
{
    parameters:
        @class(VENTOS::AddMobileNode);
        @display("i=block/network2");
        
        int headerLength = default(256bit) @unit(bit); //header length of the application  
        
        // mode = 1: for incident detection
        // mode = 2: ACC
        // mode = 3: for CACC stream or platoon
        // mode = 4: for router
        // mode = 5: platoon management
        // mode = 6: bi-directional control
        int mode = default(-1);     
        int submode = default(-1);   
        
        int numVehicles = default(10);
        string vehiclesType = default("Type_CACC1");
        int interval = default(1000ms) @unit(ms);  // insertion interval in deterministic distribution
        int lambda = default(1300);  // insertion rate [veh/h]
        
        int plnSize = default(5);
        double plnSpace = default(3.5s) @unit(s);
        
        // mode 10 and 11
        double overlap = default(0);  // increasing the overlap, moves the peak more to the left in time
        string vehRouteDist = default("70,30,0");  // through traffic, left turn, right turn
        bool vehMultiClass = default(false);  // turn on/off insertion of multi-class vehicles
        string vehClassDist = default("90,10"); // multi-class vehicle distribution (passenger vehicle (normal), emergency vehicle)
        bool bike = default(false);  // turn on/off insertion of bikes
        string bikeRouteDist = default("100,0,0");
        
        string flowFile = default("flows.xml");
        string flowSetId = default("");
}


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
        string vehId = default("");   
        double startTime = default(-1s) @unit(s);  // -1: simulation start
        
        bool warmUp = default(false);        
        double stopPosition = default(9); 
        double warmUpSpeed = default(0);  
        double waitingTime = default(15);  // waiting time after all vehicles enter the simulation        
        
        double minSpeed = default(5);
        double normalSpeed = default(20);
        double maxSpeed = default(30);
        double switchTime = default(40s) @unit(s);  // waiting time between speed change
        string trajectoryPath = default("sumo/trajectory_StopAndGo.txt");
}


simple Tracking
{
    parameters:
        @class(VENTOS::Tracking);
        @display("i=block/network2");
        
        // trackingMode = -1: disable
        // trackingMode =  0: zoom in
        // trackingMode =  1: zoom in slowly
        // trackingMode =  2: track a specific vehicle
        // trackingMode =  3: move windows frame when all vehicles exit
        int mode = default(-1);                
        
        double zoom = default(11512);
        double winOffsetX = default(0);
        double winOffsetY = default(0);
        double trackingInterval = default(1);
        string trackingV = default("veh3");       // in mode 2
        string trackingLane = default("1to2_0");  // in mode 3
        double windowsOffset = default(400);      // in mode 3
}


simple TrafficControl
{
    parameters:
        @class(VENTOS::TrafficControl);
        @display("i=block/network2");    
        
        int headerLength = default(256bit) @unit(bit); //header length of the application 
        string id = default(""); 
}

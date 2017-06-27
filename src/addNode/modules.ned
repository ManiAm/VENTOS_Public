package VENTOS.src.addNode;

simple AddNode
{
    parameters:
        @class(VENTOS::AddNode);
        @display("i=block/network2");
        
        int headerLength = default(256bit) @unit(bit); //header length of the application 
        string id = default("");      
        
        string vehicle_ModuleType = default("VENTOS.src.nodes.vehicle.vehicle");
        string vehicle_ModuleName = default("V"); 
        string vehicle_ModuleDisplayString = default("b=3,3,rect,red,black,1");  
        
        string bike_ModuleType = default("VENTOS.src.nodes.bicycle.bicycle");       
        string bike_ModuleName = default("B");        
        string bike_ModuleDisplayString = default("b=3,3,rect,yellow,black,1");
        
        string ped_ModuleType = default("VENTOS.src.nodes.pedestrian.pedestrian");      
        string ped_ModuleName = default("P");        
        string ped_ModuleDisplayString = default("i=abstract/person");  
        
        string obstacle_ModuleType = default("VENTOS.src.nodes.obstacle.obstacle");
        string obstacle_ModuleName = default("OBS"); 
        string obstacle_ModuleDisplayString = default("i=status/error");     
        
        string RSU_ModuleType = default("VENTOS.src.nodes.rsu.RSU");
        string RSU_ModuleName = default("RSU");
        string RSU_ModuleDisplayString = default("i=device/antennatower"); 
        
        string adversary_ModuleType = default("VENTOS.src.nodes.adversary.Adversary");
        string adversary_ModuleName = default("ADV");
        string adversary_ModuleDisplayString = default("i=status/red");         
        
        string CA_ModuleType = default("VENTOS.src.nodes.CA.CA");        
        string CA_ModuleName = default("CA");
        string CA_ModuleDisplayString = default("i=old/comp_a");   
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

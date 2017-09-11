package VENTOS.src.addNode;

// Interface for AddNode module
moduleinterface IBaseAddNode
{
        
}

simple AddNode like IBaseAddNode
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


simple TrafficControl
{
    parameters:
        @class(VENTOS::TrafficControl);
        @display("i=block/network2");    
        
        int headerLength = default(256bit) @unit(bit); //header length of the application 
        string id = default(""); 
}


simple gui
{
    parameters:
        @class(VENTOS::gui);
        @display("i=block/network2");
        
        int headerLength = default(256bit) @unit(bit); //header length of the application 
        string id = default("");
}

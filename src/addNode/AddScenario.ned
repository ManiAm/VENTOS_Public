
package VENTOS.src.addNode;

simple AddScenario extends AddNode
{
    parameters:
        @class(VENTOS::AddScenario);
        @display("i=block/network2");
        
        // mode = 1: for incident detection
        // mode = 2: ACC
        // mode = 3: for CACC stream or platoon
        // mode = 4: for router
        // mode = 5: platoon management
        // mode = 6: bi-directional control
        int mode = default(-1);        
        
        int totalVehicles = default(10);
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
        
        string RSUfile = default("RSUsLocation.xml");
        string flowFile = default("flows.xml");
        string flowSetId = default("");
}


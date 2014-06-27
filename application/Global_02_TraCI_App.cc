
#include "Global_02_TraCI_App.h"


Define_Module(TraCI_App);


TraCI_App::~TraCI_App()
{

}


void TraCI_App::initialize(int stage)
{
    TraCI_Extend::initialize(stage);

    if(stage == 0)
    {
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get the ptr of the AddVehicle module
        cModule *module = simulation.getSystemModule()->getSubmodule("vehicleAdd");
        AddVehiclePtr = static_cast<VehicleAdd *>(module);
        if(AddVehiclePtr == NULL)
            error("can not get a pointer to the AddVehicle module.");

        // get the ptr of the SpeedProfile module
        module = simulation.getSystemModule()->getSubmodule("speedprofile");
        SpeedProfilePtr = static_cast<SpeedProfile *>(module);
        if(SpeedProfilePtr == NULL)
            error("can not get a pointer to the SpeedProfile module.");

        // get the ptr of the Warmup module
        module = simulation.getSystemModule()->getSubmodule("warmup");
        WarmupPtr = static_cast<Warmup *>(module);
        if(WarmupPtr == NULL)
            error("can not get a pointer to the Warmup module.");

        // get the ptr of the Traffic Light module
        module = simulation.getSystemModule()->getSubmodule("trafficLight");
        tlPtr = static_cast<TrafficLight *>(module);
        if(tlPtr == NULL)
            error("can not get a pointer to the TrafficLight module.");

        // get the ptr of the Statistics module
        module = simulation.getSystemModule()->getSubmodule("statistics");
        StatPtr = static_cast<Statistics *>(module);
        if(StatPtr == NULL)
            error("can not get a pointer to the Statistics module.");

        terminate = par("terminate").doubleValue();
    }
}


void TraCI_App::handleSelfMsg(cMessage *msg)
{
    if (msg == updataGUI)
    {
        TrackingGUI();
    }
    else
    {
        TraCI_Extend::handleSelfMsg(msg);
    }
}


// this method is called once!
// in this method TraCI is up and running :)
void TraCI_App::init_traci()
{
    TraCI_Extend::init_traci();

    // create adversary node into the network
    bool adversary = par("adversary").boolValue();
    if(adversary)
        AddAdversaryModule();

    // create RSU modules in the network
    bool RSU = par("RSU").boolValue();
    if(RSU)
        AddRSUModules();

    // add vehicles dynamically into SUMO
    AddVehiclePtr->Add();

    // track vehicles in SUMO GUI
    tracking = par("tracking").boolValue();

    // todo:
    // use TraCI command, and check if we are running in GUI mode

    if(tracking)
    {
        zoom = par("zoom").doubleValue();
        if(zoom < 0)
            error("zoom value is not correct!");

        initialWindowsOffset = par("initialWindowsOffset").doubleValue();
        if(initialWindowsOffset < 0)
            error("Initial Windows Offset value is not correct!");

        trackingInterval = par("trackingInterval").doubleValue();
        if(trackingInterval <= 0)
            error("Tracking interval should be positive!");

        trackingMode = par("trackingMode").longValue();
        trackingV = par("trackingV").stdstringValue();
        trackingLane = par("trackingLane").stringValue();
        windowsOffset = par("windowsOffset").doubleValue();

        updataGUI = new cMessage("updataGUI", 1);

        // zoom-in GUI
        commandSetGUIZoom(zoom);

        // adjust Windows initially
        commandSetGUIOffset(initialWindowsOffset, 0.);

        TrackingGUI();
    }
}


void TraCI_App::AddAdversaryModule()
{
    cModule* parentMod = getParentModule();
    if (!parentMod) error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get("c3po.ned.Adversary");

    cModule* mod = nodeType->create("adversary", parentMod);
    mod->finalizeParameters();
    mod->getDisplayString().updateWith("i=old/comp_a");
    mod->buildInside();
    mod->scheduleStart(simTime());
    mod->callInitialize();
}


void TraCI_App::AddRSUModules()
{
    // ####################################
    // Step 1: read RSU locations from file
    // ####################################

    string RSUfile = par("RSUfile").stringValue();
    boost::filesystem::path RSUfilePath = SUMOfullDirectory / RSUfile;

    // check if this file is valid?
    if( !exists( RSUfilePath ) )
    {
        error("RSU file does not exist in %s", RSUfilePath.string().c_str());
    }

    deque<RSUEntry*> RSUs = commandReadRSUsCoord(RSUfilePath.string());

    // ################################
    // Step 2: create RSUs into OMNET++
    // ################################

    int NoRSUs = RSUs.size();

    cModule* parentMod = getParentModule();
    if (!parentMod) error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get("c3po.ned.RSU");

    // We only create RSUs in OMNET++ without moving them to
    // the correct coordinate
    for(int i = 0; i < NoRSUs; i++)
    {
        cModule* mod = nodeType->create("RSU", parentMod, NoRSUs, i);
        mod->finalizeParameters();
        mod->getDisplayString().updateWith("i=device/antennatower");
        mod->buildInside();
        mod->scheduleStart(simTime());
        mod->callInitialize();
    }

    // ##################################################
    // Step 3: create a polygon for radio circle in SUMO
    // ##################################################

    // get the radius of an RSU
    cModule *module = simulation.getSystemModule()->getSubmodule("RSU", 0);
    double radius = atof( module->getDisplayString().getTagArg("r",0) );

    for(int i = 0; i < NoRSUs; i++)
    {
        Coord *center = new Coord(RSUs[i]->coordX, RSUs[i]->coordY);
        commandAddCirclePoly(RSUs[i]->name, "RSU", TraCIColor::fromTkColor("blue"), center, radius);
    }
}


void TraCI_App::TrackingGUI()
{
    if(trackingMode == 1)
    {
        // get vehicle position in SUMO coordinates
        Coord co = commandGetVehiclePos(trackingV);

        if(co.x > 0)
            commandSetGUIOffset(co.x, co.y);
    }
    else if(trackingMode == 2)
    {
        // get a list of vehicles on this lane!
        list<string> myList = commandGetVehicleLaneList(trackingLane.c_str());

        if(!myList.empty())
        {
            // get iterator to the end
            list<string>::iterator it = myList.end();

            // iterator pointing to the last element
            --it;

            // first inserted vehicle on this lane
            string lastVehicleId = *it;

            Coord lastVehiclePos = commandGetVehiclePos(lastVehicleId);

            // get GUI windows boundary
            vector<double> windowsFrame = commandGetGUIBoundry();

            // vehicle goes out of frame?
            if(lastVehiclePos.x > windowsFrame[2] || lastVehiclePos.y > windowsFrame[3])
            {
                commandSetGUIOffset(windowsFrame[0] + windowsOffset, 0);
            }
        }
    }

    scheduleAt(simTime() + trackingInterval, updataGUI);
}


void TraCI_App::executeOneTimestep()
{
    EV << "### Sending Command to SUMO to perform simulation for TS = " << (getCurrentTimeMs()/1000.) << endl;

    TraCIScenarioManager::executeOneTimestep();

    EV << "### SUMO completed simulation for TS = " << (getCurrentTimeMs()/1000.) << endl;

    // write the statistics
    bool simulationDone = (simTime().dbl() >= terminate) ? true : false;
    StatPtr->executeOneTimestep(simulationDone);

    if(simulationDone)
    {
        // close TraCI connection
        commandTerminate();

        // then terminate
        endSimulation();
    }

    // check if warm-up phase is finished
    bool finished = WarmupPtr->DoWarmup();
    if (finished)
    {
        // we can start speed profiling
        SpeedProfilePtr->Change();
    }

    // run traffic light module
    tlPtr->Execute();
}


void TraCI_App::finish()
{
    TraCI_Extend::finish();
}


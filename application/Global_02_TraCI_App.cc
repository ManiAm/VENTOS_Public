
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

        terminate = par("terminate").doubleValue();

        collectVehiclesData = par("collectVehiclesData");
        collectInductionLoopData = par("collectInductionLoopData");

        tracking = par("tracking").boolValue();
        trackingV = par("trackingV").stdstringValue();
        trackingInterval = par("trackingInterval").doubleValue();

        updataGUI = new cMessage("updataGUI", 1);

        index = 1;
    }
    else if(stage == 1)
    {
        if(collectVehiclesData)
        {
            char fName [50];

            if( ev.isGUI() )
            {
                sprintf (fName, "%s.txt", "results/gui/speed-gap");
            }
            else
            {
                // get the current run number
                int currentRun = ev.getConfigEx()->getActiveRunNumber();
                sprintf (fName, "%s_%d.txt", "results/cmd/speed-gap", currentRun);
            }

            f1 = fopen (fName, "w");

            // write header
            fprintf (f1, "%-10s","index");
            fprintf (f1, "%-10s","vehicle");
            fprintf (f1, "%-12s","timeStep");
            fprintf (f1, "%-10s","speed");
            fprintf (f1, "%-12s","accel");
            fprintf (f1, "%-12s","pos");
            fprintf (f1, "%-10s","gap");
            fprintf (f1, "%-10s\n\n","timeGap");

            fflush(f1);
        }
    }
}


// this method is called once!
// TraCI is up and running :)
void TraCI_App::init_traci()
{
    TraCI_Extend::init_traci();

    // create adversary node into the network
    AddAdversaryModule();

    // create RSU modules in the network
    AddRSUModules();

    // todo:
    // making sure that platoonLeader exists in the sumo


    // todo:
    // use TraCI command, and check if we are running in gui mode

    // zoom-in GUI
    //commandSetGUIZoom(767.);

    // adjust windows
   // commandSetGUIOffset(200., 0.);

    // track the vehicle only if tracking is on
    if(tracking)
    {
        scheduleAt(simTime(), updataGUI);
    }

    // add vehicles dynamically into SUMO
    AddVehiclePtr->Add();
}


void TraCI_App::AddAdversaryModule()
{
    bool adversary = par("adversary").boolValue();

    if(!adversary)
        return;

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
    bool RSU = par("RSU").boolValue();

    if(!RSU)
        return;

    // ####################################
    // Step 1: read RSU locations from file
    // ####################################

    string RSUfile = par("RSUfile").stringValue();
    string RSUfilePath = SUMOfullDirectory + "/" + RSUfile;

    // check if the file exists!
    FILE *file = fopen(RSUfilePath.c_str(), "r");
    if (!file)
    {
        error("RSU file does not exist in %s", RSUfilePath.c_str());
    }

    deque<RSUEntry*> RSUs = commandReadRSUsCoord(RSUfilePath);

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


void TraCI_App::handleSelfMsg(cMessage *msg)
{
    if (msg == updataGUI)
    {
        Coord co = commandGetVehiclePos(trackingV);
        commandSetGUIOffset(co.x, co.y);

        scheduleAt(simTime() + trackingInterval, updataGUI);
    }
    else
    {
        TraCI_Extend::handleSelfMsg(msg);
    }
}


void TraCI_App::executeOneTimestep()
{
    EV << "### Sending Command to SUMO to perform simulation for TS = " << (getCurrentTimeMs()/1000.) << endl;

    TraCIScenarioManager::executeOneTimestep();

    EV << "### SUMO completed simulation for TS = " << (getCurrentTimeMs()/1000.) << endl;

    // collecting data from all vehicles in each timeStep and
    // then write it into a file each time
    if(collectVehiclesData)
        vehiclesData();

    // collecting induction loop data in each timeStep
    if(collectInductionLoopData)
        inductionLoops();

    if(simTime().dbl() >= terminate)
    {
        // write the collected induction loop data to
        // a file before terminating simulation
        if(collectInductionLoopData)
            writeToFile_InductionLoop();

        // send termination signal to statistics
        // statistic will perform some post-processing and
        // then terminates the simulation
        simsignal_t Signal_terminate = registerSignal("terminate");
        nodePtr->emit(Signal_terminate, 1);
    }

    bool finished = WarmupPtr->DoWarmup();

    // check if warm-up phase is finished
    if (finished)
    {
        // we can start speed profiling
        SpeedProfilePtr->Change();
    }

    // run traffic light module
    tlPtr->Execute();
}


void TraCI_App::vehiclesData()
{
    string vleaderID = "";

    // get all lanes in the network
    list<string> myList = commandGetLaneList();

    for(list<string>::iterator i = myList.begin(); i != myList.end(); ++i)
    {
        // get all vehicles on lane i
        list<string> myList2 = commandGetVehicleLaneList( i->c_str() );

        for(list<string>::reverse_iterator k = myList2.rbegin(); k != myList2.rend(); ++k)
        {
            string vID = k->c_str();
            writeToFile_PerVehicle(vID, vleaderID);
            vleaderID = k->c_str();
        }
    }

    // increase index after writing data for all vehicles
    if (commandGetNoVehicles() > 0)
        index++;
}


// vID is current vehicle, vleaderID is the leader (if present)
void TraCI_App::writeToFile_PerVehicle(string vID, string vleaderID)
{
    double speed = commandGetVehicleSpeed(vID);
    double pos = commandGetLanePosition(vID);
    double accel = commandGetVehicleAccel(vID);

    // calculate gap (if leading is present)
    double gap = -1;

    if(vleaderID != "")
    {
        gap = commandGetLanePosition(vleaderID) - commandGetLanePosition(vID) - commandGetVehicleLength(vleaderID);
    }

    // calculate timeGap (if leading is present)
    double timeGap = -1;

    if(vleaderID != "" && speed != 0)
        timeGap = gap / speed;

    // write the current vehicle data into file
    fprintf (f1, "%-10d ", index);
    fprintf (f1, "%-10s ", vID.c_str());
    fprintf (f1, "%-10.2f ", ( simTime()-updateInterval).dbl() );
    fprintf (f1, "%-10.2f ", speed);
    fprintf (f1, "%-10.2f ", accel);
    fprintf (f1, "%-10.2f ", pos);
    fprintf (f1, "%-10.2f ", gap);
    fprintf (f1, "%-10.2f \n", timeGap);

    fflush(f1);
}


void TraCI_App::inductionLoops()
{
    // get all loop detectors
    list<string> str = commandGetLoopDetectorList();

    // for each loop detector
    for (list<string>::iterator it=str.begin(); it != str.end(); ++it)
    {
        // only if this loop detector detected a vehicle
        if( commandGetLoopDetectorCount(*it) == 1 )
        {
            vector<string>  st = commandGetLoopDetectorVehicleData(*it);

            string vehicleName = st.at(0);
            double entryT = atof( st.at(2).c_str() );
            double leaveT = atof( st.at(3).c_str() );
            double speed = commandGetLoopDetectorSpeed(*it);  // vehicle speed at current moment

            int counter = findInVector(Vec_loopDetectors, (*it).c_str(), vehicleName.c_str());

            // its a new entry, so we add it.
            if(counter == -1)
            {
                LoopDetector *tmp = new LoopDetector( (*it).c_str(), vehicleName.c_str(), entryT, -1, speed, -1 );
                Vec_loopDetectors.push_back(tmp);
            }
            // if found, just update the leave time and leave speed
            else
            {
                Vec_loopDetectors[counter]->leaveTime = leaveT;
                Vec_loopDetectors[counter]->leaveSpeed = speed;
            }
        }
    }
}


void TraCI_App::writeToFile_InductionLoop()
{
    char fName2 [50];

    if( ev.isGUI() )
    {
        sprintf (fName2, "%s.txt", "results/gui/loop-detector");
    }
    else
    {
        // get the current run number
        int currentRun = ev.getConfigEx()->getActiveRunNumber();
        sprintf (fName2, "%s_%d.txt", "results/cmd/loop-detector", currentRun);
    }

    f2 = fopen (fName2, "w");

    // write header
    fprintf (f2, "%-20s","loopDetector");
    fprintf (f2, "%-20s","vehicleName");
    fprintf (f2, "%-20s","vehicleEntryTime");
    fprintf (f2, "%-20s","vehicleLeaveTime");
    fprintf (f2, "%-22s","vehicleEntrySpeed");
    fprintf (f2, "%-22s\n\n","vehicleLeaveSpeed");

    // write body
    for(unsigned int k=0; k<Vec_loopDetectors.size(); k++)
    {
        fprintf ( f2, "%-20s ", Vec_loopDetectors[k]->detectorName );
        fprintf ( f2, "%-20s ", Vec_loopDetectors[k]->vehicleName );
        fprintf ( f2, "%-20.2f ", Vec_loopDetectors[k]->entryTime );
        fprintf ( f2, "%-20.2f ", Vec_loopDetectors[k]->leaveTime );
        fprintf ( f2, "%-20.2f ", Vec_loopDetectors[k]->entrySpeed );
        fprintf ( f2, "%-20.2f ", Vec_loopDetectors[k]->leaveSpeed );
        fprintf ( f2, "\n" );
    }

    fclose(f2);
}


int TraCI_App::findInVector( vector<LoopDetector *> Vec, const char *detectorName, const char *vehicleName )
{
    unsigned int counter;
    bool found = false;

    for(counter = 0; counter < Vec.size(); counter++)
    {
        if( strcmp(Vec[counter]->detectorName, detectorName) == 0 && strcmp(Vec[counter]->vehicleName, vehicleName) == 0)
        {
            found = true;
            break;
        }
    }

    if(!found)
        return -1;
    else
        return counter;
}


void TraCI_App::finish()
{
    TraCI_Extend::finish();
    fclose(f1);
}


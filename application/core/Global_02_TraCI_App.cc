
#include "Global_02_TraCI_App.h"

namespace VENTOS {

Define_Module(VENTOS::TraCI_App);


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

        // get the ptr of the Warmup module
        cModule *module = simulation.getSystemModule()->getSubmodule("warmup");
        WarmupPtr = static_cast<Warmup *>(module);
        if(WarmupPtr == NULL)
            error("can not get a pointer to the Warmup module.");

        // get the ptr of the SpeedProfile module
        module = simulation.getSystemModule()->getSubmodule("speedprofile");
        SpeedProfilePtr = static_cast<SpeedProfile *>(module);
        if(SpeedProfilePtr == NULL)
            error("can not get a pointer to the SpeedProfile module.");

        // get the ptr of the Statistics module
        module = simulation.getSystemModule()->getSubmodule("statistics");
        StatPtr = static_cast<Statistics *>(module);
        if(StatPtr == NULL)
            error("can not get a pointer to the Statistics module.");

        terminate = par("terminate").doubleValue();
    }
}


void TraCI_App::finish()
{
    TraCI_Extend::finish();
}


void TraCI_App::handleSelfMsg(cMessage *msg)
{
    TraCI_Extend::handleSelfMsg(msg);
}


// this method is called once!
// in this method TraCI is up and running :)
void TraCI_App::init_traci()
{
    TraCI_Extend::init_traci();

    // get the ptr of the Tracking module
    cModule *module = simulation.getSystemModule()->getSubmodule("Tracking");
    Tracking *TrackingPtr = static_cast<Tracking *>(module);
    if(TrackingPtr == NULL)
        error("can not get a pointer to the Tracking module.");
    TrackingPtr->Start();  // start tracking

    // get the ptr of the AddVehicle module
    module = simulation.getSystemModule()->getSubmodule("vehicleAdd");
    VehicleAdd *AddVehiclePtr = static_cast<VehicleAdd *>(module);
    if(AddVehiclePtr == NULL)
        error("can not get a pointer to the AddVehicle module.");
    AddVehiclePtr->Add();  // add vehicles dynamically into SUMO

    // get the ptr of the AddRSU module
    module = simulation.getSystemModule()->getSubmodule("RSUAdd");
    RSUAdd *AddRSUPtr = static_cast<RSUAdd *>(module);
    if(AddRSUPtr == NULL)
        error("can not get a pointer to the AddRSU module.");
    AddRSUPtr->Add();  // add RSUs into SUMO

    // create adversary node into the network
    bool adversary = par("adversary").boolValue();
    if(adversary)
        AddAdversaryModule();
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
}


}



#include "TraCI_App.h"
#include "modules/mobility/traci/TraCIScenarioManagerInet.h"
#include "modules/mobility/traci/TraCIMobility.h"

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

        terminate = par("terminate").doubleValue();

        bikeModuleType = par("bikeModuleType").stringValue();
        bikeModuleName = par("bikeModuleName").stringValue();
        bikeModuleDisplayString = par("bikeModuleDisplayString").stringValue();

        pedModuleType = par("pedModuleType").stringValue();
        pedModuleName = par("pedModuleName").stringValue();
        pedModuleDisplayString = par("pedModuleDisplayString").stringValue();
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


// this method is called once (TraCI is up and running)
void TraCI_App::init_traci()
{
    TraCI_Extend::init_traci();

    simsignal_t Signal_executeFirstTS = registerSignal("executeFirstTS");
    nodePtr->emit(Signal_executeFirstTS, 1);
}


void TraCI_App::executeOneTimestep()
{
    EV << "### Sending Command to SUMO to perform simulation for TS = " << (getCurrentTimeMs()/1000.) << endl;

    TraCIScenarioManager::executeOneTimestep();

    EV << "### SUMO completed simulation for TS = " << (getCurrentTimeMs()/1000.) << endl;

    bool simulationDone = (simTime().dbl() >= terminate) or commandGetMinExpectedVehicles() == 0;

    simsignal_t Signal_executeEachTS = registerSignal("executeEachTS");
    nodePtr->emit(Signal_executeEachTS, (long)simulationDone);

    if(simulationDone)
    {
        // close TraCI connection
        commandTerminate();

        // then terminate
        endSimulation();
    }
}


void TraCI_App::addModule(std::string nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id, double speed, double angle)
{
    string vehType = commandGetVehicleTypeId(nodeId);

    if (vehType.find("TypeManual") != std::string::npos ||
        vehType.find("TypeACC") != std::string::npos ||
        vehType.find("TypeCACC") != std::string::npos ||
        vehType.find("TypeObstacle") != std::string::npos )
    {
        // this is default (do nothing)
        // type is "c3po.ned.vehicle"
        // name is "V"
    }
    else if (vehType == "Pedestrian")
    {
        type = pedModuleType;
        name = pedModuleName;
        displayString = pedModuleDisplayString;
    }
    else if (vehType == "Bicycle")
    {
        type = bikeModuleType;
        name =  bikeModuleName;
        displayString = bikeModuleDisplayString;
    }
    else
        error("Unknown module type in TraCI_App::addModule");

    // ########################################
    // Copy of TraCIScenarioManager::addModule
    // ########################################

    if (hosts.find(nodeId) != hosts.end()) error("tried adding duplicate module");

    if (queuedVehicles.find(nodeId) != queuedVehicles.end()) {
        queuedVehicles.erase(nodeId);
    }
    double option1 = hosts.size() / (hosts.size() + unEquippedHosts.size() + 1.0);
    double option2 = (hosts.size() + 1) / (hosts.size() + unEquippedHosts.size() + 1.0);

    if (fabs(option1 - penetrationRate) < fabs(option2 - penetrationRate)) {
        unEquippedHosts.insert(nodeId);
        return;
    }

    int32_t nodeVectorIndex = nextNodeVectorIndex++;

    cModule* parentmod = getParentModule();
    if (!parentmod) error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get(type.c_str());
    if (!nodeType) error("Module Type \"%s\" not found", type.c_str());

    //TODO: this trashes the vectsize member of the cModule, although nobody seems to use it
    cModule* mod = nodeType->create(name.c_str(), parentmod, nodeVectorIndex, nodeVectorIndex);
    mod->finalizeParameters();
    mod->getDisplayString().parse(displayString.c_str());
    mod->buildInside();
    mod->scheduleStart(simTime() + updateInterval);

    // pre-initialize TraCIMobility
    for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
        cModule* submod = iter();
        ifInetTraCIMobilityCallPreInitialize(submod, nodeId, position, road_id, speed, angle);
        TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
        if (!mm) continue;
        mm->preInitialize(nodeId, position, road_id, speed, angle);
    }

    mod->callInitialize();
    hosts[nodeId] = mod;

    // post-initialize TraCIMobility
    for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
        cModule* submod = iter();
        TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
        if (!mm) continue;
        mm->changePosition();
    }
}

}

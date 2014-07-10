
#include "Global_05_RSUAdd.h"

Define_Module(RSUAdd);


RSUAdd::~RSUAdd()
{

}


void RSUAdd::initialize(int stage)
{
    if(stage ==0)
    {
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        boost::filesystem::path SUMODirectory = simulation.getSystemModule()->par("SUMODirectory").stringValue();
        boost::filesystem::path VENTOSfullDirectory = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getConfigEntry("network").getBaseDirectory();
        SUMOfullDirectory = VENTOSfullDirectory / SUMODirectory;   // home/mani/Desktop/VENTOS/sumo/CACC_Platoon

        on = par("on").boolValue();
        mode = par("mode").longValue();
    }
}


void RSUAdd::handleMessage(cMessage *msg)
{

}


void RSUAdd::Add()
{
    // if dynamic adding is off, return
    if (!on)
        return;

    if(mode == 1)
    {
        Scenario1();
    }
}


void RSUAdd::Scenario1()
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

    deque<RSUEntry*> RSUs = TraCI->commandReadRSUsCoord(RSUfilePath.string());

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
        TraCI->commandAddCirclePoly(RSUs[i]->name, "RSU", TraCIColor::fromTkColor("blue"), center, radius);
    }
}


void RSUAdd::finish()
{


}


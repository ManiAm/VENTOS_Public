
#include "Global_05_AddAdversary.h"

namespace VENTOS {

Define_Module(VENTOS::AddAdversary);


AddAdversary::~AddAdversary()
{

}


void AddAdversary::initialize(int stage)
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

        on = par("on").boolValue();
        mode = par("mode").longValue();
    }
}


void AddAdversary::finish()
{

}


void AddAdversary::handleMessage(cMessage *msg)
{

}


void AddAdversary::Add()
{
    // if dynamic adding is off, return
    if (!on)
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


}


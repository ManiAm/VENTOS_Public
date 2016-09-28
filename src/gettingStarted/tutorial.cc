#include "tutorial.h"  // including the header file above

namespace VENTOS {

// Define_Module macro registers this class with OMNET++
Define_Module(VENTOS::tutorial);

tutorial::~tutorial()
{

}

void tutorial::initialize(int stage)
{
    if(stage == 0)
    {
        // get a pointer to the TraCI module
        cModule *module = omnetpp::getSimulation()->getSystemModule()->getSubmodule("TraCI");
        // make sure module TraCI exists
        ASSERT(module);
        // get a pointer to TraCI class
        TraCI = static_cast<TraCI_Commands *>(module);
        // make sure the TraCI pointer is not null
        ASSERT(TraCI);

        // register and subscribe to Signal_initialize_withTraCI
        Signal_initialize_withTraCI = registerSignal("initialize_withTraCI");
        omnetpp::getSimulation()->getSystemModule()->subscribe("initialize_withTraCI", this);

        // register and subscribe to Signal_executeEachTS
        Signal_executeEachTS = registerSignal("executeEachTS");
        omnetpp::getSimulation()->getSystemModule()->subscribe("executeEachTS", this);
    }
}

void tutorial::finish()
{
    // unsubscribe from initialize_withTraCI signal
    if(omnetpp::getSimulation()->getSystemModule()->isSubscribed("initialize_withTraCI", this))
        omnetpp::getSimulation()->getSystemModule()->unsubscribe("initialize_withTraCI", this);

    // unsubscribe from executeEachTS signal
    if(omnetpp::getSimulation()->getSystemModule()->isSubscribed("executeEachTS", this))
        omnetpp::getSimulation()->getSystemModule()->unsubscribe("executeEachTS", this);
}

void tutorial::handleMessage(omnetpp::cMessage *msg)
{

}

void tutorial::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long i, cObject* details)
{
    Enter_Method_Silent();

    // if Signal_executeEachTS is received, then call executeEachTimestep() method
    if(signalID == Signal_executeEachTS)
    {
        tutorial::executeEachTimestep();
    }
    // if Signal_initialize_withTraCI is received, then call initialize_withTraCI() method
    else if(signalID == Signal_initialize_withTraCI)
    {
        tutorial::initialize_withTraCI();
    }
}

void tutorial::initialize_withTraCI()
{

}

void tutorial::executeEachTimestep()
{

}

}

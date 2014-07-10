
#include "Global_09_TrafficLight.h"


Define_Module(TrafficLight);


TrafficLight::~TrafficLight()
{

}


void TrafficLight::initialize(int stage)
{
    if(stage == 0)
    {
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);
    }
}


void TrafficLight::handleMessage(cMessage *msg)
{

}


// is called in each simulation time step
void TrafficLight::Execute()
{
    EV << "simulation time is: " << simTime().dbl() << endl;

    // get all loop detectors
    list<string> str = TraCI->commandGetLoopDetectorList();

    // for each loop detector
    for (list<string>::iterator it=str.begin(); it != str.end(); ++it)
    {
        EV << "loop detector name: " << *it << endl;
    }

    // get a pointer to vehicle CACC1
    cModule *module = simulation.getSystemModule()->getSubmodule("CACC1");
    ApplVManager *car = static_cast<ApplVManager *>(module);


}


void TrafficLight::finish()
{


}



#include "TrafficLight.h"

#include <sstream>
#include <iostream>
#include <fstream>

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

        // get a pointer to the manager module
        cModule *module = simulation.getSystemModule()->getSubmodule("manager");
        manager = static_cast<TraCI_Extend *>(module);
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
    std::list<std::string> str = manager->commandGetLoopDetectorList();

    // for each loop detector
    for (std::list<std::string>::iterator it=str.begin(); it != str.end(); ++it)
    {
        EV << "loop detector name: " << *it << endl;
    }

    // get a pointer to vehicle CACC1
    cModule *module = simulation.getSystemModule()->getSubmodule("CACC1");
    ApplVSumoInteraction *car = static_cast<ApplVSumoInteraction *>(module);


}


void TrafficLight::finish()
{


}


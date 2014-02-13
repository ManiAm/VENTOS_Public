
#include "Warmup.h"

#include <sstream>
#include <iostream>
#include <fstream>

Define_Module(Warmup);


Warmup::~Warmup()
{

}


void Warmup::initialize(int stage)
{
    if(stage ==0)
    {
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

        // get a pointer to the manager module
        cModule *module = simulation.getSystemModule()->getSubmodule("manager");
        manager = static_cast<TraCI_Extend *>(module);

        // get the ptr of the AddVehicle module
        module = simulation.getSystemModule()->getSubmodule("addvehicle");
        AddVehiclePtr = static_cast<AddVehicle *>(module);
        if(AddVehiclePtr == NULL)
            error("can not get a pointer to the AddVehicle module.");

        totalVehicles = AddVehiclePtr->par("totalVehicles").longValue();

        on = par("on").boolValue();
        leadingVehicle = par("leadingVehicle").stringValue(); // todo: make it automatic
        stopPosition = par("stopPosition").doubleValue();
        waitingTime = par("waitingTime").doubleValue();

        startTime = -1;
        IsWarmUpFinished = false;
        warmupFinish = new cMessage("warmupFinish", 1);
    }
}


void Warmup::handleMessage(cMessage *msg)
{
    if (msg == warmupFinish)
    {
        IsWarmUpFinished = true;
    }
}


bool Warmup::DoWarmup()
{
    // if warmup is not on, return finished == true
    if (!on)
        return true;

    if(IsWarmUpFinished)
        return true;

    if( warmupFinish->isScheduled() )
        return false;

    // upon first call, store current simulation time as startTime
    if(startTime == -1)
    {
        startTime = simTime().dbl();
    }

    if(startTime < 0)
        error("startTime is less than 0 in Warmup.");

    double pos = manager->commandGetLanePosition(leadingVehicle);

    if(pos >= stopPosition)
    {
        // start breaking at stopPosition, and stop (waiting for other vehicles)
        manager->commandSetSpeed(leadingVehicle, 0.);

        // get # of vehicles that have entered simulation so far
        int n = manager->commandGetNoVehicles();

        // if all vehicles are in the simulation
        if( n == totalVehicles )
        {
            scheduleAt(simTime() + waitingTime, warmupFinish);
        }
    }

    return false;
}


void Warmup::finish()
{


}


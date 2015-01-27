
#include "Global_07_VehicleWarmup.h"

namespace VENTOS {

Define_Module(VENTOS::Warmup);


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

        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

        // get totoal vehicles from AddVehicle module
        totalVehicles = simulation.getSystemModule()->getSubmodule("addVehicle")->par("totalVehicles").longValue();

        on = par("on").boolValue();
        laneId = par("laneId").stringValue();
        stopPosition = totalVehicles * par("stopPosition").doubleValue();
        waitingTime = par("waitingTime").doubleValue();

        startTime = -1;
        IsWarmUpFinished = false;

        if(on)
        {
            warmupFinish = new cMessage("warmupFinish", 1);
        }
    }
}


void Warmup::finish()
{


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

    // who is leading?
    list<string> veh = TraCI->commandGetLaneVehicleList(laneId);

    if(veh.empty())
        return false;

    string leadingVehicle = veh.back();

    double pos = TraCI->commandGetVehicleLanePosition(leadingVehicle);

    if(pos >= stopPosition)
    {
        // start breaking at stopPosition, and stop (waiting for other vehicles)
        TraCI->commandChangeVehicleSpeed(leadingVehicle, 0.);

        // get # of vehicles that have entered simulation so far
        int n = TraCI->commandGetVehicleCount();

        // if all vehicles are in the simulation
        if( n == totalVehicles )
        {
            scheduleAt(simTime() + waitingTime, warmupFinish);
        }
    }

    return false;
}

}


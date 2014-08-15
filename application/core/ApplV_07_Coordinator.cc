
#include "ApplV_07_Coordinator.h"


namespace VENTOS {

Define_Module(VENTOS::ApplVCoordinator);

ApplVCoordinator::~ApplVCoordinator()
{

}


void ApplVCoordinator::initialize(int stage)
{
    ApplVPlatoonMg::initialize(stage);

    if (stage == 0)
    {
        coordination = new cMessage("coordination timer", KIND_TIMER);
        scheduleAt(simTime(), coordination);
    }
}


void ApplVCoordinator::finish()
{
    ApplVPlatoonMg::finish();
}


void ApplVCoordinator::handleSelfMsg(cMessage* msg)
{
    ApplVPlatoonMg::handleSelfMsg(msg);

    if(msg == coordination)
    {
        coordinator();
    }
}


void ApplVCoordinator::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down
    ApplVPlatoonMg::onBeaconVehicle(wsm);
}


void ApplVCoordinator::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down
    ApplVPlatoonMg::onBeaconRSU(wsm);
}


void ApplVCoordinator::onData(PlatoonMsg* wsm)
{
    // pass it down
    ApplVPlatoonMg::onData(wsm);
}


// is called, every time the position of vehicle changes
void ApplVCoordinator::handlePositionUpdate(cObject* obj)
{
    ApplVPlatoonMg::handlePositionUpdate(obj);
}


void ApplVCoordinator::coordinator()
{
    // merge and split (youtube)
    // scenario1();

    // leave (youtube)
    //scenario2();

    // measure merge, split, leader/follower leave duration for
    // a platoon of size 10
    scenario3();


    scheduleAt(simTime() + 0.1, coordination);
}


// merge and split
void ApplVCoordinator::scenario1()
{
    if(simTime().dbl() >= 37)
    {
        optPlnSize = 13;
    }

    if(simTime().dbl() >= 77)
    {
        optPlnSize = 4;
    }

    if(simTime().dbl() >= 94)
    {
        optPlnSize = 10;
    }

    if(simTime().dbl() >= 131)
    {
        optPlnSize = 3;
    }
    if(simTime().dbl() >= 188)
    {
        optPlnSize = 2;
    }
}


// leave
void ApplVCoordinator::scenario2()
{
    // leader leaves
    if(simTime().dbl() == 26)
    {
        if(SUMOvID == "CACC1")
        {
            ApplVPlatoonMg::leavePlatoon();
        }
    }

    // last follower leaves
    if(simTime().dbl() == 67)
    {
        if(SUMOvID == "CACC5")
        {
            ApplVPlatoonMg::leavePlatoon();
        }
    }

    // middle follower leaves
    if(simTime().dbl() == 120)
    {
        if(SUMOvID == "CACC2")
        {
            ApplVPlatoonMg::leavePlatoon();
        }
    }
}


void ApplVCoordinator::scenario3()
{
    // get the current run number
    int currentRun = ev.getConfigEx()->getActiveRunNumber();

    if(currentRun == 0)
    {
        if(simTime().dbl() == 40)
        {
            TraCI->commandSetSpeed("CACC1", 20.);
        }
        else if(simTime().dbl() == 73)
        {
            // disable automatic merging
            mergeEnabled = false;

            if(SUMOvID == "CACC1")
                splitFromPlatoon(9);
        }
        else if(simTime().dbl() == 118)
        {
            // enable automatic merging
            mergeEnabled = true;
        }
    }
    else if(currentRun == 1)
    {
        if(simTime().dbl() == 40)
        {
            TraCI->commandSetSpeed("CACC1", 20.);
        }
        else if(simTime().dbl() == 73)
        {
            // disable automatic merging
            mergeEnabled = false;

            if(SUMOvID == "CACC1")
                splitFromPlatoon(8);
        }
        else if(simTime().dbl() == 118)
        {
            // enable automatic merging
            mergeEnabled = true;
        }
    }
    else if(currentRun == 2)
    {
        if(simTime().dbl() == 40)
        {
            TraCI->commandSetSpeed("CACC1", 20.);
        }
        else if(simTime().dbl() == 73)
        {
            // disable automatic merging
            mergeEnabled = false;

            if(SUMOvID == "CACC1")
                splitFromPlatoon(7);
        }
        else if(simTime().dbl() == 118)
        {
            // enable automatic merging
            mergeEnabled = true;
        }
    }
    else if(currentRun == 3)
    {
        if(simTime().dbl() == 40)
        {
            TraCI->commandSetSpeed("CACC1", 20.);
        }
        else if(simTime().dbl() == 73)
        {
            // disable automatic merging
            mergeEnabled = false;

            if(SUMOvID == "CACC1")
                splitFromPlatoon(6);
        }
        else if(simTime().dbl() == 118)
        {
            // enable automatic merging
            mergeEnabled = true;
        }
    }
    else if(currentRun == 4)
    {
        if(simTime().dbl() == 40)
        {
            TraCI->commandSetSpeed("CACC1", 20.);
        }
        else if(simTime().dbl() == 73)
        {
            // disable automatic merging
            mergeEnabled = false;

            if(SUMOvID == "CACC1")
                splitFromPlatoon(5);
        }
        else if(simTime().dbl() == 118)
        {
            // enable automatic merging
            mergeEnabled = true;
        }
    }
    else if(currentRun == 5)
    {
        if(simTime().dbl() == 40)
        {
            TraCI->commandSetSpeed("CACC1", 20.);
        }
        else if(simTime().dbl() == 73)
        {
            // disable automatic merging
            mergeEnabled = false;

            if(SUMOvID == "CACC1")
                splitFromPlatoon(4);
        }
        else if(simTime().dbl() == 118)
        {
            // enable automatic merging
            mergeEnabled = true;
        }
    }
    else if(currentRun == 6)
    {
        if(simTime().dbl() == 40)
        {
            TraCI->commandSetSpeed("CACC1", 20.);
        }
        else if(simTime().dbl() == 73)
        {
            // disable automatic merging
            mergeEnabled = false;

            if(SUMOvID == "CACC1")
                splitFromPlatoon(3);
        }
        else if(simTime().dbl() == 118)
        {
            // enable automatic merging
            mergeEnabled = true;
        }
    }
    else if(currentRun == 7)
    {
        if(simTime().dbl() == 40)
        {
            TraCI->commandSetSpeed("CACC1", 20.);
        }
        else if(simTime().dbl() == 73)
        {
            // disable automatic merging
            mergeEnabled = false;

            if(SUMOvID == "CACC1")
                splitFromPlatoon(2);
        }
        else if(simTime().dbl() == 118)
        {
            // enable automatic merging
            mergeEnabled = true;
        }
    }
    else if(currentRun == 8)
    {
        if(simTime().dbl() == 40)
        {
            TraCI->commandSetSpeed("CACC1", 20.);
        }
        else if(simTime().dbl() == 73)
        {
            // disable automatic merging
            mergeEnabled = false;

            if(SUMOvID == "CACC1")
                splitFromPlatoon(1);
        }
        else if(simTime().dbl() == 118)
        {
            // enable automatic merging
            mergeEnabled = true;
        }
    }
    // ----------------------------------------
    // leave
    // ----------------------------------------
    else if(currentRun == 9)
    {
        if(simTime().dbl() == 40)
        {
            TraCI->commandSetSpeed("CACC1", 20.);
        }
        else if(simTime().dbl() == 73)
        {
            // leader leaves
            if(SUMOvID == "CACC1")
                leavePlatoon();
        }
        else if(simTime().dbl() == 118)
        {
            // last follower leaves
            if(SUMOvID == "CACC10")
                leavePlatoon();
        }
    }
    else if(currentRun == 10)
    {
        if(simTime().dbl() == 40)
        {
            TraCI->commandSetSpeed("CACC1", 20.);
        }
        else if(simTime().dbl() == 73)
        {
            if(SUMOvID == "CACC3")
                leavePlatoon();
        }
        else if(simTime().dbl() == 118)
        {
            if(SUMOvID == "CACC6")
                leavePlatoon();
        }
    }
    else if(currentRun == 11)
    {
        if(simTime().dbl() == 40)
        {
            TraCI->commandSetSpeed("CACC1", 20.);
        }
        else if(simTime().dbl() == 73)
        {
            if(SUMOvID == "CACC7")
                leavePlatoon();
        }
        else if(simTime().dbl() == 118)
        {
            if(SUMOvID == "CACC5")
                leavePlatoon();
        }
    }
    else if(currentRun == 12)
    {
        if(simTime().dbl() == 40)
        {
            TraCI->commandSetSpeed("CACC1", 20.);
        }
        else if(simTime().dbl() == 73)
        {
            if(SUMOvID == "CACC8")
                leavePlatoon();
        }
        else if(simTime().dbl() == 118)
        {
            if(SUMOvID == "CACC4")
                leavePlatoon();
        }
    }
}


} // end of namespace

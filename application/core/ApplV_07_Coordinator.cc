
#include "ApplV_07_Coordinator.h"


namespace VENTOS {

double ApplVCoordinator::stopTime = 100000;

Define_Module(VENTOS::ApplVCoordinator);

ApplVCoordinator::~ApplVCoordinator()
{

}


void ApplVCoordinator::initialize(int stage)
{
    ApplVPlatoonMg::initialize(stage);

    if (stage == 0 && plnMode == 3)
    {
        coordinationMode = par("coordinationMode").longValue();

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
    // do nothing!
    if(coordinationMode == 1)
    {

    }
    // video: three plns (1,5,5) doing merge/split
    else if(coordinationMode == 2)
    {
        scenario2();
    }
    // video: leader/last follower/middle follower leave
    else if(coordinationMode == 3)
    {
        scenario3();
    }
    // video: IEEE CSS competition
    else if(coordinationMode == 4)
    {
        scenario4();
    }
    // speed profile of platoon members in split/merge
    else if(coordinationMode == 5)
    {
        scenario5();
    }
    // measure merge, split, leader/follower leave duration for
    // a platoon of size 10
    else if(coordinationMode == 6)
    {
        scenario6();
    }
    // effect of changing Tp on merge/split duration
    else if(coordinationMode == 7)
    {
        scenario7();
    }
    else if(coordinationMode == 8)
    {
        scenario8();
    }
    else
        error("not a valid coordination mode!");

    scheduleAt(simTime() + 0.1, coordination);
}

// -----------------------------------------------------------

void ApplVCoordinator::scenario2()
{
    if(simTime().dbl() == 37)
    {
        TraCI->commandSetSpeed("CACC1", 20.);
    }
    else if(simTime().dbl() == 59)
    {
        optPlnSize = 13;
    }
    else if(simTime().dbl() == 94)
    {
        optPlnSize = 4;
    }
    else if(simTime().dbl() == 131)
    {
        optPlnSize = 10;
    }
    else if(simTime().dbl() == 188)
    {
        optPlnSize = 3;
    }
    else if(simTime().dbl() == 200)
    {
        optPlnSize = 2;
    }
}


void ApplVCoordinator::scenario3()
{
    if(simTime().dbl() == 26)
    {
        TraCI->commandSetSpeed("CACC1", 20.);
    }
    // leader leaves
    else if(simTime().dbl() == 49)
    {
        if(SUMOvID == "CACC1")
        {
            ApplVPlatoonMg::leavePlatoon();
        }
    }
    // last follower leaves
    else if(simTime().dbl() == 68)
    {
        if(SUMOvID == "CACC6")
        {
            ApplVPlatoonMg::leavePlatoon();
        }
    }
    // middle follower leaves
    else if(simTime().dbl() == 80)
    {
        if(SUMOvID == "CACC3")
        {
            ApplVPlatoonMg::leavePlatoon();
        }
    }
}


void ApplVCoordinator::scenario4()
{
    if(TraCI->commandGetNoVehicles() == 10)
        stopTime = simTime().dbl();

    // all vehicles entering after the stopTime
    // are background traffic
    if(entryTime > stopTime)
    {
        entryEnabled = false;
        VANETenabled = false;
    }

    if(simTime().dbl() == 40)
    {
        TraCI->commandSetSpeed("CACC1", 20.);
        TraCI->commandSetSpeed("CACC6", 20.);
    }
    else if(simTime().dbl() == 55)
    {
        optPlnSize = 10;
    }
    else if(simTime().dbl() == 77)
    {
        optPlnSize = 4;
    }
    else if(simTime().dbl() == 100)
    {
        optPlnSize = 10;
    }
    // leader leaves
    else if(simTime().dbl() == 140)
    {
        if(SUMOvID == "CACC1")
        {
            ApplVPlatoonMg::leavePlatoon();
        }
    }
//    // last follower leaves
//    else if(simTime().dbl() == 160)
//    {
//        if(SUMOvID == "CACC5")
//        {
//            ApplVPlatoonMg::leavePlatoon();
//        }
//    }
//    // middle follower leaves
//    else if(simTime().dbl() == 190)
//    {
//        if(SUMOvID == "CACC2")
//        {
//            ApplVPlatoonMg::leavePlatoon();
//        }
//    }
}


void ApplVCoordinator::scenario5()
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


void ApplVCoordinator::scenario6()
{
    if(ev.isGUI())
        error("Run coordination mode 3 in command-line, not GUI!");

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


void ApplVCoordinator::scenario7()
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


void ApplVCoordinator::scenario8()
{
    if(simTime().dbl() == 40)
    {
        optPlnSize = 10;
        TraCI->commandSetSpeed("CACC1", 20.);
    }
    else if(simTime().dbl() == 73)
    {
        // disable automatic merging
        mergeEnabled = false;

        optPlnSize = 2;
    }
    else if(simTime().dbl() == 130)
    {
        // enable automatic merging
        mergeEnabled = true;
        optPlnSize = 10;
    }
}


} // end of namespace

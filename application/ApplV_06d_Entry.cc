
#include "ApplV_06_PlatoonMg.h"


void ApplVPlatoonMg::entry_handleSelfMsg(cMessage* msg)
{
    // start the Entry maneuver
    if(msg == entryManeuverEvt && vehicleState == state_idle)
    {
        TraCI->commandSetvClass(SUMOvID, "vip");   // change vClass

        int32_t bitset = TraCI->commandMakeLaneChangeMode(10, 01, 01, 01, 01);
        TraCI->commandSetLaneChangeMode(SUMOvID, bitset);  // alter 'lane change' mode
        TraCI->commandChangeLane(SUMOvID, 1, 5);   // change to lane 1 (special lane)

        TraCI->commandSetSpeed(SUMOvID, 5.);       // set speed to 5 m/s

        // change state to waitForLaneChange
        vehicleState = state_waitForLaneChange;
        reportStateToStat();

        scheduleAt(simTime() + 0.1, plnTIMER0);
    }
    else if(msg == plnTIMER0 && vehicleState == state_waitForLaneChange)
    {
        // check if we change lane
        if(TraCI->commandGetLaneIndex(SUMOvID) == 1)
        {
            // make it a free agent
            plnID = SUMOvID;
            myPlnDepth = 0;
            plnSize = 1;
            plnMembersList.push_back(SUMOvID);
            TraCI->commandSetTg(SUMOvID, 3.5);

            busy = false;

            // change color to red!
            TraCIColor newColor = TraCIColor::fromTkColor("red");
            TraCI->getCommandInterface()->setColor(SUMOvID, newColor);

            // change state to platoon leader
            vehicleState = state_platoonLeader;
            reportStateToStat();
        }
        else
            scheduleAt(simTime() + 0.1, plnTIMER0);
    }
}


void ApplVPlatoonMg::entry_BeaconFSM(BeaconVehicle* wsm)
{


}


void ApplVPlatoonMg::entry_DataFSM(PlatoonMsg *wsm)
{


}


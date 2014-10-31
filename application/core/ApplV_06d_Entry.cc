
#include "ApplV_06_PlatoonMg.h"

namespace VENTOS {

void ApplVPlatoonMg::entry_handleSelfMsg(cMessage* msg)
{
    if(!entryEnabled)
        return;

    // start the Entry maneuver
    if(msg == entryManeuverEvt && vehicleState == state_idle)
    {
        // check if we are at lane 0
        if(TraCI->commandGetLaneIndex(SUMOvID) == 0)
        {
            TraCI->commandSetvClass(SUMOvID, "vip");   // change vClass to vip

            int32_t bitset = TraCI->commandMakeLaneChangeMode(10, 01, 01, 01, 01);
            TraCI->commandSetLaneChangeMode(SUMOvID, bitset);   // alter 'lane change' mode
            TraCI->commandChangeLane(SUMOvID, 1, 5);   // change to lane 1 (special lane)

            // change state to waitForLaneChange
            vehicleState = state_waitForLaneChange;
            reportStateToStat();

            scheduleAt(simTime() + 0.1, plnTIMER0);
        }
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
            TraCI->commandSetTg(SUMOvID, TP);

            busy = false;

            // get my leading vehicle
            vector<string> vleaderIDnew = TraCI->commandGetLeading(SUMOvID, sonarDist);
            string vleaderID = vleaderIDnew[0];

            // if no leading, set speed to 5 m/s
            if(vleaderID == "")
            {
                TraCI->commandSetSpeed(SUMOvID, 5.);
            }
            // if I have leading, set speed to max
            else
            {
                TraCI->commandSetSpeed(SUMOvID, 30.);
            }

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

}

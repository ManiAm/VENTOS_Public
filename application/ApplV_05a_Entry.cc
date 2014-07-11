
#include "ApplV_05_PlatoonMg.h"


void ApplVPlatoonMg::entry_handleSelfMsg(cMessage* msg)
{
    // start the Entry maneuver
    if(msg == entryManeuverEvt)
    {
        entry_FSM();
    }
}


void ApplVPlatoonMg::entry_FSM()
{
    if(vehicleState == state_idle)
    {
        TraCI->commandSetvClass(SUMOvID, "vip");   // change vClass

        int32_t bitset = TraCI->commandMakeLaneChangeMode(10, 01, 01, 01, 01);
        TraCI->commandSetLaneChangeMode(SUMOvID, bitset);  // lane change mode
        TraCI->commandChangeLane(SUMOvID, 1, 5);   // change to lane 1 (special lane)

        TraCI->commandSetSpeed(SUMOvID, 5.);       // set speed to 5 m/s

        // make it a free agent
        plnID = SUMOvID;
        myPlnDepth = 0;
        plnSize = 1;
        plnMembersList.push_back(SUMOvID);
        TraCI->commandSetTg(SUMOvID, 3.5);

        // myPlnDepth has changed! update the color
        updateColor();

        // change state to platoon leader
        vehicleState = state_platoonLeader;

        // report to statistics
        CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState));
        simsignal_t Signal_VehicleState = registerSignal("VehicleState");
        nodePtr->emit(Signal_VehicleState, state);
    }
}



#include "ApplV_05_PlatoonMg.h"


void ApplVPlatoonMg::entry_handleSelfMsg(cMessage* msg)
{
    // start the Entry maneuver
    if(msg == entryManeuverEvt && vehicleState == state_idle)
    {
        // start monitoring beacons from lane 1
        vehicleState = state_monitoring;

        // report to statistics
        CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
        simsignal_t Signal_VehicleState = registerSignal("VehicleState");
        nodePtr->emit(Signal_VehicleState, state);

        scheduleAt(simTime() + 1., plnTIMER0);
    }
    else if(msg == plnTIMER0)
    {
        // is it safe to change lane?
        if( SafeToChangeLane() )
        {
            TraCI->commandSetvClass(SUMOvID, "vip");   // change vClass

            int32_t bitset = TraCI->commandMakeLaneChangeMode(10, 01, 01, 01, 01);
            TraCI->commandSetLaneChangeMode(SUMOvID, bitset);  // alter 'lane change' mode
            TraCI->commandChangeLane(SUMOvID, 1, 5);   // change to lane 1 (special lane)

            TraCI->commandSetSpeed(SUMOvID, 5.);       // set speed to 5 m/s

            // change state to waitForLaneChange
            vehicleState = state_waitForLaneChange;

            // report to statistics
            CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
            simsignal_t Signal_VehicleState = registerSignal("VehicleState");
            nodePtr->emit(Signal_VehicleState, state);

            scheduleAt(simTime() + 0.1, plnTIMER0a);
        }
        else
        {
            leastDistFront = DBL_MAX;
            leastDistBack = DBL_MAX;

            scheduleAt(simTime() + 1., plnTIMER0);
        }
    }
    else if(msg == plnTIMER0a && vehicleState == state_waitForLaneChange)
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

            // myPlnDepth has changed! update the color
            updateColor();

            // change state to platoon leader
            vehicleState = state_platoonLeader;

            // report to statistics
            CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
            simsignal_t Signal_VehicleState = registerSignal("VehicleState");
            nodePtr->emit(Signal_VehicleState, state);
        }
        else
            scheduleAt(simTime() + 0.1, plnTIMER0a);
    }
}


void ApplVPlatoonMg::entry_BeaconFSM(BeaconVehicle* wsm)
{
    if(vehicleState == state_monitoring && string(wsm->getLane()) == "1to2_1")
    {
        Coord cord = TraCI->commandGetVehiclePos(SUMOvID);
        double dist = cord.x - wsm->getPos().x;

        // beacon is from the front vehicle
        if(dist <= 0 && fabs(dist) < leastDistFront)
        {
            leastFront = new NearestVehicle(wsm->getSender(), wsm->getPlatoonDepth(), dist-TraCI->commandGetVehicleLength(SUMOvID));
            leastDistFront = fabs(dist);
        }
        // beacon is from the back vehicle
        else if(dist > 0 && dist < leastDistBack)
        {
            leastBack = new NearestVehicle(wsm->getSender(), wsm->getPlatoonDepth(), dist-TraCI->commandGetVehicleLength(SUMOvID));
            leastDistBack = dist;
        }
    }
}


bool ApplVPlatoonMg::SafeToChangeLane()
{
    // no beacons! it is safe :)
    if(leastFront == NULL && leastBack == NULL)
    {
        return true;
    }
    // only beacon from back
    else if(leastFront == NULL && leastBack != NULL && leastBack->depth == 0 && leastBack->dist > 10 )
    {
        return true;
    }
    // only beacon from front
    else if(leastBack == NULL && leastFront != NULL && leastFront->dist > 10)
    {
        return true;
    }
    // beacons from both front and back
    else if(leastFront != NULL && leastFront->dist > 10 && leastBack != NULL && leastBack->depth == 0 && leastBack->dist > 10)
    {
        return true;
    }
    else
        return false;
}



#include "ApplV_05_PlatoonMg.h"


void ApplVPlatoonMg::split_handleSelfMsg(cMessage* msg)
{

}


void ApplVPlatoonMg::split_BeaconFSM(BeaconVehicle *wsm)
{


}


void ApplVPlatoonMg::split_DataFSM(PlatoonMsg *wsm)
{
    if(vehicleState == state_sendSplitReq)
    {

        scheduleAt(simTime() + 1., plnTIMER4);
    }


}


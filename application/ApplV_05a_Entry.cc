
#include "ApplV_05_PlatoonMg.h"


void ApplVPlatoonMg::entryManeuver()
{
    if(vehicleState == state_idle)
    {
        TraCI->commandSetvClass(SUMOvID, "vip");   // change vClass
        TraCI->commandChangeLane(SUMOvID, 1, 5);   // change to lane 1 (special lane)
        TraCI->commandSetSpeed(SUMOvID, 20.);     // set speed to 20 m/s
    }



}


void ApplVPlatoonMg::entryFSM()
{



}





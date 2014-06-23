
#include "ApplV_05_PlatoonMg.h"


void ApplVPlatoonMg::entryManeuver()
{
    if(vehicleState == state_idle)
    {
        // go to the special lane
        TraCI->commandSetvClass(SUMOvID, "ignoring");
        TraCI->commandChangeLane(SUMOvID, 0, 5);   // change to lane 0
        TraCI->getCommandInterface()->setSpeed(SUMOvID, 0.);
    }



}

void ApplVPlatoonMg::entryFSM()
{



}





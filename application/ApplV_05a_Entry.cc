
#include "ApplV_05_PlatoonMg.h"


void ApplVPlatoonMg::entryManeuver()
{
    if(vehicleState == state_idle)
    {
        TraCI->commandSetvClass(SUMOvID, "vip");   // change vClass
        TraCI->commandChangeLane(SUMOvID, 1, 5);   // change to lane 1 (special lane)
        TraCI->commandSetSpeed(SUMOvID, 20.);      // set speed to 20 m/s

        plnID = SUMOvID;
        myPlnDepth = 0;
        plnSize = 1;
        plnMembersList.push_back(SUMOvID);

        // change the color to red
        TraCIColor newColor = TraCIColor::fromTkColor("red");
        TraCI->getCommandInterface()->setColor(SUMOvID, newColor);
    }



}


void ApplVPlatoonMg::entryFSM()
{



}





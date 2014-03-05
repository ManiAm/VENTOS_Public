
#include "ApplVPlatoon.h"

Define_Module(ApplVPlatoon);

void ApplVPlatoon::initialize(int stage)
{
    ApplVBeacon::initialize(stage);

	if (stage == 0)
	{
        one_vehicle_look_ahead = par("one_vehicle_look_ahead");
        platoonID = par("platoonID").stringValue();

        if(platoonID == SUMOvID)
        {
            myPlatoonDepth = 0;
        }
        // we are not part of a platoon (yet!)
        else
        {
            myPlatoonDepth = -1;
            platoonID = "";
        }
	}
}


void ApplVPlatoon::handleLowerMsg(cMessage* msg)
{
    error("ApplVPlatoon should not receive any lower message!");
}


void ApplVPlatoon::handleSelfMsg(cMessage* msg)
{
    if(one_vehicle_look_ahead)
    {
        ApplVBeacon::handleSelfMsg(msg);
        return;
    }

    if (msg->getKind() == SEND_BEACON_EVT)
    {
        Beacon* Msg = ApplVBeacon::prepareBeacon();

        // fill-in the related fields to platoon
        Msg->setPlatoonID(platoonID.c_str());
        Msg->setPlatoonDepth(myPlatoonDepth);

        EV << "## Created beacon msg for vehicle: " << SUMOvID << std::endl;
        ApplVBeacon::printBeaconContent(Msg);

        // send it
        sendDelayedDown(Msg,individualOffset);

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, sendBeaconEvt);
    }
}


void ApplVPlatoon::onBeacon(Beacon* wsm)
{
    // I am platoon leader and I do not care about the received beacon!
    if(myPlatoonDepth == 0)
        return;

    // I am not part of any platoon yet!
    if(platoonID == "")
    {
        if( std::string(wsm->getPlatoonID()) != "" && wsm->getPlatoonDepth() == 0 )
        {
            EV << "This beacon is from a platoon leader. I will join ..." << std::endl;
            platoonID = wsm->getPlatoonID();

            // change the color to blue
            TraCIColor newColor = TraCIColor::fromTkColor("blue");
            manager->commandSetVehicleColor(SUMOvID, newColor);
        }
    }
    // platoonID != "" which means I am already part of a platoon
    // if the beacon is from my platoon leader
    else if( isBeaconFromMyPlatoonLeader(wsm) )
    {
        // do nothing!
        EV << "This beacon is from my platoon leader ..." << std::endl;
    }
    // I received a beacon from another platoon
    else if( std::string(wsm->getPlatoonID()) != platoonID )
    {
        // ignore the beacon msg
    }
}


// is called, every time the position of vehicle changes
void ApplVPlatoon::handlePositionUpdate(cObject* obj)
{
    ApplVBeacon::handlePositionUpdate(obj);
}


bool ApplVPlatoon::isBeaconFromMyPlatoonLeader(Beacon* wsm)
{
    // check if a platoon leader is sending this
    if( wsm->getPlatoonDepth() == 0 )
    {
        // check if this is actually my platoon leader
        if( std::string(wsm->getPlatoonID()) == platoonID)
        {
            return true;
        }
    }

    return false;
}


// get the gap to the leading vehicle using sonar
double ApplVPlatoon::getGap(std::string vleaderID)
{
    if(vleaderID == "")
        return -1;

    double gap = manager->commandGetLanePosition(vleaderID) - manager->commandGetLanePosition(SUMOvID) - manager->commandGetVehicleLength(vleaderID);

    return gap;
}


bool ApplVPlatoon::isBeaconFromLeading(Beacon* wsm)
{
    // step 1: check if a leading vehicle is present

    std::string vleaderID = manager->commandGetLeading_M(SUMOvID);

    if(vleaderID == "")
    {
        EV << "This vehicle has no leading vehicle." << std::endl;
        return false;
    }

    // step 2: is it on the same lane?

    std::string myLane = manager->commandGetLaneId(SUMOvID);
    std::string beaconLane = wsm->getLane();

    EV << "I am on lane " << manager->commandGetLaneId(SUMOvID) << ", and other vehicle is on lane " << wsm->getLane() << std::endl;

    if( myLane != beaconLane )
    {
        EV << "Not on the same lane!" << std::endl;
        return false;
    }

    EV << "We are on the same lane!" << std::endl;

    // step 3: is the distance equal to gap?

    Coord cord = manager->commandGetVehiclePos(SUMOvID);
    double dist = sqrt( pow(cord.x - wsm->getPos().x, 2) +  pow(cord.y - wsm->getPos().y, 2) );

    // subtract the length of the leading vehicle from dist
    dist = dist - manager->commandGetVehicleLength(vleaderID);

    // use our sonar to compute the gap
    double gap = getGap(vleaderID);

    EV << "my coord (x,y): " << cord.x << "," << cord.y << std::endl;
    EV << "other coord (x,y): " << wsm->getPos().x << "," << wsm->getPos().y << std::endl;
    EV << "distance is " << dist << ", and gap is " << gap << std::endl;

    double diff = fabs(dist - gap);

    if(diff > 0.001)
    {
        EV << "distance does not match the gap!" << std::endl;
        return false;
    }

    EV << "This beacon is from the leading vehicle!" << std::endl;
    return true;
}


void ApplVPlatoon::finish()
{

}


ApplVPlatoon::~ApplVPlatoon()
{

}


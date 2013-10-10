
#include "ApplV.h"

Define_Module(ApplV);

void ApplV::initialize(int stage)
{
    BaseApplV::initialize(stage);

	if (stage == 0)
	{
		annotations = AnnotationManagerAccess().getIfExists();
		ASSERT(annotations);

		sentMessage = false;
		lastDroveAt = simTime();
	}
}


void ApplV::onBeacon(WaveShortMessage* wsm)
{
    // vehicles other than CACC should ignore the received beacon
    if(SUMOvType != "TypeCACC")
        return;

    DBG << "## Received beacon ..." << std::endl;
    printMsg(wsm);

    DBG << "Check if beacon is from leading vehicle ..." << std::endl;
    bool result = isBeaconFromLeader(wsm);

    // update the parameters of this vehicle in SUMO
    if(result)
        updateParamsSumo(wsm);



    /*
    DBG << "Received beacon priority  " << wsm->getPriority() << " at " << simTime() << std::endl;
    int senderId = wsm->getSenderAddress();

    if (sendData)
    {
        t_channel channel = dataOnSch ? type_SCH : type_CCH;
        sendWSM(prepareWSM("data", dataLengthBits, channel, dataPriority, senderId,2));
    }
    */
}


void ApplV::onData(WaveShortMessage* wsm)
{
	findHost()->getDisplayString().updateWith("r=16,green");
	annotations->scheduleErase(1, annotations->drawLine(wsm->getPos(), traci->getPositionAt(simTime()), "blue"));

	if (traci->getRoadId()[0] != ':') traci->commandChangeRoute(wsm->getWsmData(), 9999);
	if (!sentMessage) sendMessage(wsm->getWsmData());
}


void ApplV::sendMessage(std::string blockedRoadId)
{
	sentMessage = true;

	t_channel channel = dataOnSch ? type_SCH : type_CCH;
	WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
	wsm->setWsmData(blockedRoadId.c_str());
	sendWSM(wsm);
}


// is called, every time the position of vehicle changes
void ApplV::handlePositionUpdate(cObject* obj)
{
    BaseApplV::handlePositionUpdate(obj);

	// stopped for at least 10s?
	if (traci->getSpeed() < 1)
	{
		if (simTime() - lastDroveAt >= 10)
		{
			findHost()->getDisplayString().updateWith("r=16,red");
			if (!sentMessage) sendMessage(traci->getRoadId());
		}
	}
	else
	{
		lastDroveAt = simTime();
	}
}


std::string  ApplV::getLeader()
{
    // get the lane id (like 1to2_0)
    std::string laneID = manager->commandGetLaneId(SUMOvID);

    // get a list of all vehicles on this lane (left to right)
    std::list<std::string> list = manager->commandGetVehiclesOnLane(laneID);

    std::string vleaderID = "";

    for(std::list<std::string>::reverse_iterator i = list.rbegin(); i != list.rend(); ++i)
    {
        std::string currentID = i->c_str();

        if(currentID == SUMOvID)
            return vleaderID;

        vleaderID = i->c_str();
    }

    return "";
}


// get the gap to the leading vehicle using sonar
double ApplV::getGap(std::string vleaderID)
{
    if(vleaderID == "")
        return -1;

    std::string leaderType = manager->commandGetVehicleType(vleaderID);
    double gap = manager->commandGetLanePosition(vleaderID) - manager->commandGetLanePosition(SUMOvID) - manager->commandGetVehicleLength(leaderType);

    return gap;
}


bool ApplV::isBeaconFromLeader(WaveShortMessage* wsm)
{
    // step 1: check if a leading vehicle is present

    // use our sonar to compute the gap
    double gap = getGap(getLeader());

    if(gap == -1)
    {
        DBG << "This vehicle has no leading vehicle (gap = -1)" << std::endl;
        return false;
    }

    // step 2: is it on the same lane?

    std::string myLane = manager->commandGetLaneId(SUMOvID);
    std::string beaconLane = wsm->getLane();

    DBG << "I am on lane " << manager->commandGetLaneId(SUMOvID) << ", and other vehicle is on lane " << wsm->getLane() << std::endl;

    if( myLane != beaconLane )
    {
        DBG << "Not on the same lane!" << std::endl;
        return false;
    }

    DBG << "We are on the same lane!" << std::endl;

    // step 3: is the distance equal to gap?

    Coord cord = manager->commandGetVehiclePos(SUMOvID);
    double dist = sqrt( pow(cord.x - wsm->getPos().x, 2) +  pow(cord.y - wsm->getPos().y, 2) );

    // subtract the length of the leading vehicle from dist
    std::string vleaderType = manager->commandGetVehicleType(getLeader());
    dist = dist - manager->commandGetVehicleLength(vleaderType);

    DBG << "distance is " << dist << ", and gap is " << gap << std::endl;

    double diff = abs(dist - gap);

    if(diff > 0.01)
    {
        DBG << "distance does not match the gap!" << std::endl;
        return false;
    }

    DBG << "This beacon is from the leading vehicle!" << std::endl;
    return true;
}


void ApplV::updateParamsSumo(WaveShortMessage* wsm)
{
    // update the


}




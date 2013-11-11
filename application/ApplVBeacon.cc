
#include "ApplVBeacon.h"

Define_Module(ApplVBeacon);

void ApplVBeacon::initialize(int stage)
{
    ApplVBase::initialize(stage);

	if (stage == 0)
	{
        // beaconing parameters
        sendBeacons = par("sendBeacons").boolValue();
        beaconInterval = par("beaconInterval").doubleValue();
        maxOffset = par("maxOffset").doubleValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        // simulate asynchronous channel access
        double offSet = dblrand() * (beaconInterval/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);

        if (sendBeacons && SUMOvType == "TypeCACC")
        {
            scheduleAt(simTime() + offSet, sendBeaconEvt);
        }

		annotations = AnnotationManagerAccess().getIfExists();
		ASSERT(annotations);

		sentMessage = false;
		lastDroveAt = simTime();
	}
}


void ApplVBeacon::handleLowerMsg(cMessage* msg)
{
    ApplVBase::handleLowerMsg(msg);

    // make sure msg is of type WaveShortMessage
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()) == "beacon")
    {
        onBeacon(wsm);
    }
    else if (std::string(wsm->getName()) == "data")
    {
        onData(wsm);
    }
}


void ApplVBeacon::handleSelfMsg(cMessage* msg)
{
    ApplVBase::handleSelfMsg(msg);

    switch (msg->getKind())
    {
        case SEND_BEACON_EVT:
        {
            WaveShortMessage* beaconMsg = prepareBeacon("beacon", beaconLengthBits, type_CCH, beaconPriority, 0);

            DBG << "## Created beacon msg for vehicle: " << SUMOvID << std::endl;
            printBeaconContent(beaconMsg);

            // send it
            sendWSM(beaconMsg);

            // schedule for next beacon broadcast
            scheduleAt(simTime() + beaconInterval, sendBeaconEvt);

            break;
        }
    }
}


WaveShortMessage*  ApplVBeacon::prepareBeacon(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial)
{
    if (SUMOvType != "TypeCACC")
    {
        throw cRuntimeError("Only CACC vehicles can send beacon!");
    }

    WaveShortMessage* wsm = new WaveShortMessage(name.c_str());

    wsm->addBitLength(headerLength);
    wsm->addBitLength(lengthBits);

    wsm->setWsmVersion(1);
    wsm->setSecurityType(1);

    switch (channel)
    {
        case type_SCH: wsm->setChannelNumber(Channels::SCH1); break; //will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
        case type_CCH: wsm->setChannelNumber(Channels::CCH); break;
    }

    wsm->setDataRate(1);
    wsm->setPriority(priority);
    wsm->setPsid(0);

    // wsm->setSerial(serial);
    // wsm->setTimestamp(simTime());

    // fill in the sender/receiver fields
    wsm->setSender(SUMOvID.c_str());
    wsm->setRecipient("broadcast");

    // get the current position (from sumo)
    Coord cord = manager->commandGetVehiclePos(SUMOvID);
    wsm->setPos(cord);

    // get current speed from sumo
    wsm->setSpeed( manager->commandGetVehicleSpeed(SUMOvID) );

    // get current acceleration
    wsm->setAccel( manager->commandGetVehicleAccel(SUMOvID) );

    // get maxDecel of vehicle
    std::string vType = manager->commandGetVehicleType(SUMOvID);
    wsm->setMaxDecel( manager->commandGetVehicleMaxDecel(vType) );

    // get current lane
    wsm->setLane( manager->commandGetLaneId(SUMOvID).c_str() );

    return wsm;
}


// print beacon fields (for debugging purposes)
void ApplVBeacon::printBeaconContent(WaveShortMessage* wsm)
{
    DBG << wsm->getWsmVersion() << " | ";
    DBG << wsm->getSecurityType() << " | ";
    DBG << wsm->getChannelNumber() << " | ";
    DBG << wsm->getDataRate() << " | ";
    DBG << wsm->getPriority() << " | ";
    DBG << wsm->getPsid() << " | ";
    DBG << wsm->getPsc() << " | ";
    DBG << wsm->getWsmLength() << " | ";
    DBG << wsm->getWsmData() << " ||| ";

    DBG << wsm->getSender() << " | ";
    DBG << wsm->getRecipient() << " | ";
    DBG << wsm->getPos() << " | ";
    DBG << wsm->getSpeed() << " | ";
    DBG << wsm->getAccel() << " | ";
    DBG << wsm->getMaxDecel() << " | ";
    DBG << wsm->getLane() << " | ";
    DBG << wsm->getPlatoonID() << " | ";
    DBG << wsm->getIsPlatoonLeader() << std::endl;
}


void ApplVBeacon::onBeacon(WaveShortMessage* wsm)
{
    // vehicles other than CACC should ignore the received beacon
    if(SUMOvType != "TypeCACC")
        return;

    // ignore the received beacon (testing mode switch)
    if(simTime().dbl() >= 39 && SUMOvID == "CACC3")
    {
        //return;
    }

    DBG << "## " << SUMOvID << " received beacon ..." << std::endl;
    printBeaconContent(wsm);

    DBG << "Check if beacon is from leading vehicle ..." << std::endl;
    bool result = isBeaconFromLeading(wsm);

    // update the parameters of this vehicle in SUMO
    if(result)
    {
        // set the speed
        manager->commandSetLeading(0x15, SUMOvID, (double)wsm->getSpeed());
        DBG << "Leading vehicle speed = " << (double)wsm->getSpeed() << std::endl;

        // set the Accel
        manager->commandSetLeading(0x17, SUMOvID, (double)wsm->getAccel());
        DBG << "Leading vehicle accel = " << (double)wsm->getAccel() << std::endl;

        // set the MaxDecel
        manager->commandSetLeading(0x16, SUMOvID, (double)wsm->getMaxDecel());
        DBG << "Leading vehicle maxDecel = " << (double)wsm->getMaxDecel() << std::endl;
    }


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


void ApplVBeacon::onData(WaveShortMessage* wsm)
{
	findHost()->getDisplayString().updateWith("r=16,green");
	annotations->scheduleErase(1, annotations->drawLine(wsm->getPos(), traci->getPositionAt(simTime()), "blue"));

	if (traci->getRoadId()[0] != ':') traci->commandChangeRoute(wsm->getWsmData(), 9999);
	if (!sentMessage) sendMessage(wsm->getWsmData());
}


void ApplVBeacon::sendMessage(std::string blockedRoadId)
{
	sentMessage = true;

	//t_channel channel = dataOnSch ? type_SCH : type_CCH;
	//WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
	//wsm->setWsmData(blockedRoadId.c_str());
	//sendWSM(wsm);
}


// is called, every time the position of vehicle changes
void ApplVBeacon::handlePositionUpdate(cObject* obj)
{
    ApplVBase::handlePositionUpdate(obj);

	// stopped for at least 10s?
	if (traci->getSpeed() < 1)
	{
		if (simTime() - lastDroveAt >= 10)
		{
			findHost()->getDisplayString().updateWith("r=16,red");
			//if (!sentMessage) sendMessage(traci->getRoadId());
		}
	}
	else
	{
		lastDroveAt = simTime();
	}
}


std::string  ApplVBeacon::getLeading()
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
double ApplVBeacon::getGap(std::string vleaderID)
{
    if(vleaderID == "")
        return -1;

    std::string leaderType = manager->commandGetVehicleType(vleaderID);
    double gap = manager->commandGetLanePosition(vleaderID) - manager->commandGetLanePosition(SUMOvID) - manager->commandGetVehicleLength(leaderType);

    return gap;
}


bool ApplVBeacon::isBeaconFromLeading(WaveShortMessage* wsm)
{
    // step 1: check if a leading vehicle is present

    // use our sonar to compute the gap
    double gap = getGap(getLeading());

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
    std::string vleaderType = manager->commandGetVehicleType(getLeading());
    dist = dist - manager->commandGetVehicleLength(vleaderType);

    DBG << "my coord (x,y): " << cord.x << "," << cord.y << std::endl;
    DBG << "other coord (x,y): " << wsm->getPos().x << "," << wsm->getPos().y << std::endl;
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


void ApplVBeacon::sendWSM(WaveShortMessage* wsm)
{
    sendDelayedDown(wsm,individualOffset);
}


void ApplVBeacon::finish()
{
    if (sendBeaconEvt->isScheduled())
    {
        cancelAndDelete(sendBeaconEvt);
    }
    else
    {
        delete sendBeaconEvt;
    }

    findHost()->unsubscribe(mobilityStateChangedSignal, this);
}


ApplVBeacon::~ApplVBeacon()
{

}



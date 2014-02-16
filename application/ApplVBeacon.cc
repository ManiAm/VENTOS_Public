
#include "ApplVBeacon.h"

Define_Module(ApplVBeacon);

void ApplVBeacon::initialize(int stage)
{
    ApplVBase::initialize(stage);

	if (stage == 0)
	{
        // NED variables (beaconing parameters)
        sendBeacons = par("sendBeacons").boolValue();
        beaconInterval = par("beaconInterval").doubleValue();
        maxOffset = par("maxOffset").doubleValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        // NED variables (packet loss ratio)
        droppT = par("droppT").doubleValue();
        droppV = par("droppV").stringValue();
        plr = par("plr").doubleValue();

        // NED variable
        modeSwitch = par("modeSwitch").boolValue();

        // Class variables

        // simulate asynchronous channel access
        double offSet = dblrand() * (beaconInterval/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);
        if (sendBeacons && isCACC() )
        {
            scheduleAt(simTime() + offSet, sendBeaconEvt);
        }

        // set modeSwitch parameter in Sumo
        manager->commandSetModeSwitch(SUMOvID, modeSwitch);
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
        // vehicles other than CACC should ignore the received beacon
        if( isCACC() )
        {
            if( !dropBeacon(droppT, droppV, plr) )
            {
                ApplVBeacon::onBeacon(wsm);
            }
            // drop the beacon, and report it to statistics
            else
            {
                bool result = isBeaconFromLeading(wsm);

                if(result)
                {
                    // a beacon from preceding vehicle is drooped
                    simsignal_t Signal_beaconD = registerSignal("beaconD");
                    nodePtr->emit(Signal_beaconD, 1);
                }
                else
                {
                    // a beacon from other vehicle is drooped
                    simsignal_t Signal_beaconD = registerSignal("beaconD");
                    nodePtr->emit(Signal_beaconD, 2);
                }
            }
        }
    }
    else
    {
        error("Unknown message received in ApplVBeacon!");
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

            EV << "## Created beacon msg for vehicle: " << SUMOvID << std::endl;
            printBeaconContent(beaconMsg);

            // send it
            sendDelayedDown(beaconMsg,individualOffset);

            // schedule for next beacon broadcast
            scheduleAt(simTime() + beaconInterval, sendBeaconEvt);

            break;
        }
    }
}


WaveShortMessage*  ApplVBeacon::prepareBeacon(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial)
{
    if ( !isCACC() )
    {
        error("Only CACC vehicles can send beacon!");
    }

    WaveShortMessage* wsm = new WaveShortMessage(name.c_str());

    // add header length
    wsm->addBitLength(headerLength);

    // add payload length
    wsm->addBitLength(lengthBits);

    wsm->setWsmVersion(1);
    wsm->setSecurityType(1);

    switch (channel)
    {
        // will be rewritten at Mac1609_4 to actual Service Channel.
        // This is just so no controlInfo is needed
        case type_SCH: wsm->setChannelNumber(Channels::SCH1); break;
        case type_CCH: wsm->setChannelNumber(Channels::CCH); break;
    }

    wsm->setDataRate(1);
    wsm->setPriority(priority);
    wsm->setPsid(0);

    // wsm->setSerial(serial);
    // wsm->setTimestamp(simTime());

    // fill in the sender/receiver fields
    wsm->setSender(myFullId);
    wsm->setRecipient("broadcast");

    // set current position
    Coord cord = manager->commandGetVehiclePos(SUMOvID);
    wsm->setPos(cord);

    // set current speed
    wsm->setSpeed( manager->commandGetVehicleSpeed(SUMOvID) );

    // set current acceleration
    wsm->setAccel( manager->commandGetVehicleAccel(SUMOvID) );

    // set maxDecel
    wsm->setMaxDecel( manager->commandGetVehicleMaxDecel(SUMOvID) );

    // set current lane
    wsm->setLane( manager->commandGetLaneId(SUMOvID).c_str() );

    return wsm;
}


// print beacon fields (for debugging purposes)
void ApplVBeacon::printBeaconContent(WaveShortMessage* wsm)
{
    EV << wsm->getWsmVersion() << " | ";
    EV << wsm->getSecurityType() << " | ";
    EV << wsm->getChannelNumber() << " | ";
    EV << wsm->getDataRate() << " | ";
    EV << wsm->getPriority() << " | ";
    EV << wsm->getPsid() << " | ";
    EV << wsm->getPsc() << " | ";
    EV << wsm->getWsmLength() << " | ";
    EV << wsm->getWsmData() << " ||| ";

    EV << wsm->getSender() << " | ";
    EV << wsm->getRecipient() << " | ";
    EV << wsm->getPos() << " | ";
    EV << wsm->getSpeed() << " | ";
    EV << wsm->getAccel() << " | ";
    EV << wsm->getMaxDecel() << " | ";
    EV << wsm->getLane() << " | ";
    EV << wsm->getPlatoonID() << " | ";
    EV << wsm->getIsPlatoonLeader() << std::endl;
}


// simulate packet loss in application layer
bool ApplVBeacon::dropBeacon(double time, std::string vehicle, double plr)
{
    if(simTime().dbl() >= time)
    {
        // vehicle == "" --> drop beacon in all vehicles
        // vehicle == SUMOvID --> drop beacon only in specified vehicle
        if (vehicle == "" || vehicle == SUMOvID)
        {
            // random number in [0,1)
            double p = dblrand();

            if( p < (plr/100) )
                return true;   // drop the beacon
            else
                return false;  // keep the beacon
        }
        else
            return false;
    }
    else
        return false;
}


void ApplVBeacon::onBeacon(WaveShortMessage* wsm)
{
    EV << "## " << SUMOvID << " received beacon ..." << std::endl;
    printBeaconContent(wsm);

    EV << "Check if beacon is from leading vehicle ..." << std::endl;
    bool result = isBeaconFromLeading(wsm);

    // beacon is from leading vehicle
    // 1) update the parameters of this vehicle in SUMO
    // 2) signal to statistics
    if(result)
    {
        char buffer [100];
        sprintf (buffer, "%f#%f#%f#%f", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (simTime().dbl())*1000);
        manager->commandSetPreceding(SUMOvID, buffer);

        // a beacon from the proceeding vehicle is received
        data *pair = new data(wsm->getSender());
        simsignal_t Signal_beaconP = registerSignal("beaconP");
        nodePtr->emit(Signal_beaconP, pair);
    }
    else
    {
        // a beacon from other vehicles is received
        data *pair = new data(wsm->getSender());
        simsignal_t Signal_beaconO = registerSignal("beaconO");
        nodePtr->emit(Signal_beaconO, pair);
    }
}


void ApplVBeacon::onData(WaveShortMessage* wsm)
{
    error("ApplVBeacon can not handle data. Something is wrong!");
}


// is called, every time the position of vehicle changes
void ApplVBeacon::handlePositionUpdate(cObject* obj)
{
    ApplVBase::handlePositionUpdate(obj);
}


// get the gap to the leading vehicle using sonar
double ApplVBeacon::getGap(std::string vleaderID)
{
    if(vleaderID == "")
        return -1;

    double gap = manager->commandGetLanePosition(vleaderID) - manager->commandGetLanePosition(SUMOvID) - manager->commandGetVehicleLength(vleaderID);

    return gap;
}


bool ApplVBeacon::isBeaconFromLeading(WaveShortMessage* wsm)
{
    // step 1: check if a leading vehicle is present

    std::string vleaderID = manager->commandGetLeading_M(SUMOvID);

    // use our sonar to compute the gap
    double gap = getGap(vleaderID);

    if(gap == -1)
    {
        EV << "This vehicle has no leading vehicle (gap = -1)" << std::endl;
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
}


ApplVBeacon::~ApplVBeacon()
{

}



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

        // Class variables

        // simulate asynchronous channel access
        double offSet = dblrand() * (beaconInterval/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);
        if (sendBeacons && VANETenabled )
        {
            scheduleAt(simTime() + offSet, sendBeaconEvt);
        }
	}
}


void ApplVBeacon::handleLowerMsg(cMessage* msg)
{
    error("ApplVBeacon should not receive any lower message!");
}


void ApplVBeacon::handleSelfMsg(cMessage* msg)
{
    ApplVBase::handleSelfMsg(msg);

    if (msg->getKind() == SEND_BEACON_EVT)
    {
        Beacon* beaconMsg = prepareBeacon();

        EV << "## Created beacon msg for vehicle: " << SUMOvID << std::endl;
        printBeaconContent(beaconMsg);

        // send it
        sendDelayedDown(beaconMsg,individualOffset);

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, sendBeaconEvt);
    }
}


Beacon*  ApplVBeacon::prepareBeacon()
{
    if (!VANETenabled)
    {
        error("Only VANETenabled vehicles can send beacon!");
    }

    Beacon* wsm = new Beacon("beacon");

    // add header length
    wsm->addBitLength(headerLength);

    // add payload length
    wsm->addBitLength(beaconLengthBits);

    wsm->setWsmVersion(1);
    wsm->setSecurityType(1);

    wsm->setChannelNumber(Channels::CCH);

    wsm->setDataRate(1);
    wsm->setPriority(beaconPriority);
    wsm->setPsid(0);

    // wsm->setSerial(serial);
    // wsm->setTimestamp(simTime());

    // fill in the sender/receiver fields
    wsm->setSender(SUMOvID.c_str());
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
void ApplVBeacon::printBeaconContent(Beacon* wsm)
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
    EV << wsm->getPlatoonDepth() << std::endl;
}


void ApplVBeacon::onBeacon(Beacon* wsm)
{
    error("ApplVBeacon should not receive any beacon!");
}


// is called, every time the position of vehicle changes
void ApplVBeacon::handlePositionUpdate(cObject* obj)
{
    ApplVBase::handlePositionUpdate(obj);
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


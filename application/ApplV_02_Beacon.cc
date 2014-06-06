
#include "ApplV_02_Beacon.h"

Define_Module(ApplVBeacon);

void ApplVBeacon::initialize(int stage)
{
    ApplVBase::initialize(stage);

	if (stage == 0)
	{
	    // NED
	    if(SUMOvType != "TypeObstacle")
	    {
            VANETenabled = par("VANETenabled").boolValue();
	    }
	    else
	    {
	        VANETenabled = false;
	    }

        mode = par("mode").longValue();

        // NED variables (beaconing parameters)
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
        if (sendBeacons && VANETenabled && SUMOvType != "TypeObstacle")
        {
            scheduleAt(simTime() + offSet, sendBeaconEvt);
        }

        pauseBeaconing = false;

        platoonID = "";
        myPlatoonDepth = -1;
        platoonSize = -1;
        queue.clear();

        // pre-defined platoon
        if(mode == 2)
        {
            preDefinedPlatoonID = par("preDefinedPlatoonID").stringValue();

            // I am the platoon leader
            if(SUMOvID == preDefinedPlatoonID)
            {
                platoonID = SUMOvID;
                myPlatoonDepth = 0;
                // platoonSize;
                // queue;
            }
        }
	}
}


void ApplVBeacon::handleSelfMsg(cMessage* msg)
{
    ApplVBase::handleSelfMsg(msg);

    if (msg->getKind() == SEND_BEACON_EVT)
    {
        if(!pauseBeaconing)
        {
            BeaconVehicle* beaconMsg = prepareBeacon();

            if(mode == 1 || mode == 2 || mode == 3)
            {
                // fill-in the related fields to platoon
                beaconMsg->setPlatoonID(platoonID.c_str());
                beaconMsg->setPlatoonDepth(myPlatoonDepth);
            }

            EV << "## Created beacon msg for vehicle: " << SUMOvID << endl;
            ApplVBeacon::printBeaconContent(beaconMsg);

            // send it
            sendDelayedDown(beaconMsg,individualOffset);
        }

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, sendBeaconEvt);
    }
}


BeaconVehicle*  ApplVBeacon::prepareBeacon()
{
    if (!VANETenabled)
    {
        error("Only VANETenabled vehicles can send beacon!");
    }

    BeaconVehicle* wsm = new BeaconVehicle("beaconVehicle");

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
    Coord cord = TraCI->commandGetVehiclePos(SUMOvID);
    wsm->setPos(cord);

    // set current speed
    wsm->setSpeed( TraCI->commandGetVehicleSpeed(SUMOvID) );

    // set current acceleration
    wsm->setAccel( TraCI->commandGetVehicleAccel(SUMOvID) );

    // set maxDecel
    wsm->setMaxDecel( TraCI->commandGetVehicleMaxDecel(SUMOvID) );

    // set current lane
    wsm->setLane( TraCI->commandGetLaneId(SUMOvID).c_str() );

    return wsm;
}


// print beacon fields (for debugging purposes)
void ApplVBeacon::printBeaconContent(BeaconVehicle* wsm)
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
    EV << wsm->getPlatoonDepth() << endl;
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


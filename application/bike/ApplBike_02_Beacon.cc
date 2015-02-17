
#include "ApplBike_02_Beacon.h"

namespace VENTOS {

Define_Module(VENTOS::ApplBikeBeacon);

ApplBikeBeacon::~ApplBikeBeacon()
{

}


void ApplBikeBeacon::initialize(int stage)
{
    ApplBikeBase::initialize(stage);

	if (stage == 0)
	{
	    // NED
        VANETenabled = par("VANETenabled").boolValue();

        // NED variables (beaconing parameters)
        sendBeacons = par("sendBeacons").boolValue();
        beaconInterval = par("beaconInterval").doubleValue();
        maxOffset = par("maxOffset").doubleValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        // NED variables (data parameters)
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        // simulate asynchronous channel access
        double offSet = dblrand() * (beaconInterval/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        BicycleBeaconEvt = new cMessage("BeaconEvt", KIND_TIMER);
        if (VANETenabled)
        {
            scheduleAt(simTime() + offSet, BicycleBeaconEvt);
        }
	}
}


void ApplBikeBeacon::finish()
{
    ApplBikeBase::finish();

    if (BicycleBeaconEvt->isScheduled())
    {
        cancelAndDelete(BicycleBeaconEvt);
    }
    else
    {
        delete BicycleBeaconEvt;
    }
}


void ApplBikeBeacon::handleSelfMsg(cMessage* msg)
{
    ApplBikeBase::handleSelfMsg(msg);

    if (msg == BicycleBeaconEvt)
    {
        if(VANETenabled && sendBeacons)
        {
            BeaconBicycle* beaconMsg = ApplBikeBeacon::prepareBeacon();

            EV << "## Created beacon msg for bicycle: " << SUMObID << endl;
            ApplBikeBeacon::printBeaconContent(beaconMsg);

            // send it
            sendDelayed(beaconMsg, individualOffset, lowerLayerOut);
        }

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, BicycleBeaconEvt);
    }
}


BeaconBicycle*  ApplBikeBeacon::prepareBeacon()
{
    BeaconBicycle* wsm = new BeaconBicycle("beaconBicycle");

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
    wsm->setSender(SUMObID.c_str());
    wsm->setRecipient("broadcast");

    // set current position
    Coord cord = TraCI->commandGetVehiclePos(SUMObID);
    wsm->setPos(cord);

    // set current speed
    wsm->setSpeed( TraCI->commandGetVehicleSpeed(SUMObID) );

    // set current acceleration
    wsm->setAccel( TraCI->commandGetVehicleAccel(SUMObID) );

    // set maxDecel
    wsm->setMaxDecel( TraCI->commandGetVehicleMaxDecel(SUMObID) );

    // set current lane
    wsm->setLane( TraCI->commandGetVehicleLaneId(SUMObID).c_str() );

    return wsm;
}


// print beacon fields (for debugging purposes)
void ApplBikeBeacon::printBeaconContent(BeaconBicycle* wsm)
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


// is called, every time the position of bicycle changes
void ApplBikeBeacon::handlePositionUpdate(cObject* obj)
{
    ApplBikeBase::handlePositionUpdate(obj);
}

}


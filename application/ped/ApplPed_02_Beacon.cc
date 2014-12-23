
#include "ApplPed_02_Beacon.h"

namespace VENTOS {

Define_Module(VENTOS::ApplPedBeacon);

ApplPedBeacon::~ApplPedBeacon()
{

}


void ApplPedBeacon::initialize(int stage)
{
    ApplPedBase::initialize(stage);

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

        VehicleBeaconEvt = new cMessage("BeaconEvt", KIND_TIMER);
        if (VANETenabled)
        {
            scheduleAt(simTime() + offSet, VehicleBeaconEvt);
        }
	}
}


void ApplPedBeacon::finish()
{
    ApplPedBase::finish();

    if (VehicleBeaconEvt->isScheduled())
    {
        cancelAndDelete(VehicleBeaconEvt);
    }
    else
    {
        delete VehicleBeaconEvt;
    }
}


void ApplPedBeacon::handleSelfMsg(cMessage* msg)
{
    ApplPedBase::handleSelfMsg(msg);

    if (msg == VehicleBeaconEvt)
    {
        if(VANETenabled && sendBeacons)
        {
            BeaconVehicle* beaconMsg = prepareBeacon();

            EV << "## Created beacon msg for vehicle: " << SUMOvID << endl;
            ApplPedBeacon::printBeaconContent(beaconMsg);

            // send it
            sendDelayed(beaconMsg, individualOffset, lowerLayerOut);
        }

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, VehicleBeaconEvt);
    }
}


BeaconVehicle*  ApplPedBeacon::prepareBeacon()
{
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
    wsm->setLane( TraCI->getCommandInterface()->getLaneId(SUMOvID).c_str() );

    return wsm;
}


// print beacon fields (for debugging purposes)
void ApplPedBeacon::printBeaconContent(BeaconVehicle* wsm)
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
void ApplPedBeacon::handlePositionUpdate(cObject* obj)
{
    ApplPedBase::handlePositionUpdate(obj);
}

}


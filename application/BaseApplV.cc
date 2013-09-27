
#include "BaseApplV.h"

const simsignalwrap_t BaseApplV::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

void BaseApplV::initialize(int stage)
{
	BaseApplLayer::initialize(stage);

	if (stage==0)
	{
		myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(getParentModule());
		assert(myMac);

		myId = getParentModule()->getIndex();

		headerLength = par("headerLength").longValue();
		double maxOffset = par("maxOffset").doubleValue();
		sendBeacons = par("sendBeacons").boolValue();
		beaconLengthBits = par("beaconLengthBits").longValue();
		beaconPriority = par("beaconPriority").longValue();

		sendData = par("sendData").boolValue();
		dataLengthBits = par("dataLengthBits").longValue();
		dataOnSch = par("dataOnSch").boolValue();
		dataPriority = par("dataPriority").longValue();

		sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);

	    findHost()->subscribe(mobilityStateChangedSignal, this);

        traci = TraCIMobilityAccess().get(getParentModule());
        manager = FindModule<MyTraCI*>::findGlobalModule();

		//simulate asynchronous channel access
		double offSet = dblrand() * (par("beaconInterval").doubleValue()/2);
		offSet = offSet + floor(offSet/0.050)*0.050;
		individualOffset = dblrand() * maxOffset;

		if (sendBeacons)
		{
			scheduleAt(simTime() + offSet, sendBeaconEvt);
		}
	}
}


void BaseApplV::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj)
{
    Enter_Method_Silent();

    if (signalID == mobilityStateChangedSignal)
    {
        handlePositionUpdate(obj);
    }
}


void BaseApplV::handlePositionUpdate(cObject* obj)
{
    ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
}


void BaseApplV::handleLowerMsg(cMessage* msg)
{
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
    else
    {
        DBG << "unknown message (" << wsm->getName() << ")  received\n";
    }

    delete(msg);
}


void BaseApplV::handleSelfMsg(cMessage* msg)
{
    switch (msg->getKind())
    {
        case SEND_BEACON_EVT:
        {
            sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0));
            scheduleAt(simTime() + par("beaconInterval").doubleValue(), sendBeaconEvt);
            break;
        }
        default:
        {
            if (msg)
                DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
            break;
        }
    }
}


void BaseApplV::sendWSM(WaveShortMessage* wsm)
{
    sendDelayedDown(wsm,individualOffset);
}


WaveShortMessage*  BaseApplV::prepareWSM(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial)
{
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

	wsm->setSenderAddress(myId);
	wsm->setRecipientAddress(rcvId);

	// get the current position (from sumo)
    std::string vID = traci->getExternalId();
	Coord cord = manager->commandGetVehiclePos(vID);
	wsm->setPos(cord);

	// get current speed from sumo
	wsm->setSpeed( manager->commandGetVehicleSpeed(vID) );

	// get maxDecel of vehicle
	std::string vType = manager->commandGetVehicleType(vID);
	wsm->setMaxDecel( manager->commandGetVehicleMaxDecel(vType) );

	// get current lane
    wsm->setLane( manager->commandGetLaneId(vID).c_str() );

	// wsm->setSerial(serial);
    //wsm->setTimestamp(simTime());

    DBG << "## Created  " << wsm->getName() << " msg ..." << std::endl;
    printMsg(wsm);

	return wsm;
}


// print beacon fields (for debugging purposes)
void BaseApplV::printMsg(WaveShortMessage* wsm)
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

    DBG << wsm->getSenderAddress() << " | ";
    DBG << wsm->getRecipientAddress() << " | ";
    DBG << wsm->getPos() << " | ";
    DBG << wsm->getSpeed() << " | ";
    DBG << wsm->getMaxDecel() << " | ";
    DBG << wsm->getLane() << std::endl;
}


void BaseApplV::finish()
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


BaseApplV::~BaseApplV()
{

}



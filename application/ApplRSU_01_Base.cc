
#include "ApplRSU_01_Base.h"

Define_Module(ApplRSUBase);


void ApplRSUBase::initialize(int stage)
{
	BaseApplLayer::initialize(stage);

	if (stage==0)
	{
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

		myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(getParentModule());
		assert(myMac);

		TraCI = FindModule<TraCI_Extend*>::findGlobalModule();

        headerLength = par("headerLength").longValue();

        // NED variables (beaconing parameters)
        sendBeacons = par("sendBeacons").boolValue();
        beaconInterval = par("beaconInterval").doubleValue();
        maxOffset = par("maxOffset").doubleValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        // vehicle id in omnet++
		myId = getParentModule()->getIndex();
		myFullId = getParentModule()->getFullName();

        // simulate asynchronous channel access
        double offSet = dblrand() * (beaconInterval/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        RSUBeaconEvt = new cMessage("RSUBeaconEvt", KIND_TIMER);
        if (sendBeacons)
        {
            scheduleAt(simTime() + offSet, RSUBeaconEvt);
        }
	}
}


void ApplRSUBase::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj)
{
    Enter_Method_Silent();

}


void ApplRSUBase::handleSelfMsg(cMessage* msg)
{
    if (msg == RSUBeaconEvt)
    {
        BeaconRSU* beaconMsg = prepareBeacon();

        EV << "## Created beacon msg for " << myFullId << endl;
        printBeaconContent(beaconMsg);

        // send it
        sendDelayed(beaconMsg, individualOffset, lowerLayerOut);

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, RSUBeaconEvt);
    }
}


BeaconRSU* ApplRSUBase::prepareBeacon()
{
    BeaconRSU* wsm = new BeaconRSU("beaconRSU");

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
    wsm->setSender(myFullId);
    wsm->setRecipient("broadcast");

    // set current position
    Coord *SUMOpos = TraCI->commandGetRSUsCoord(myId);
    wsm->setPos(*SUMOpos);

    return wsm;
}


// print beacon fields (for debugging purposes)
void ApplRSUBase::printBeaconContent(BeaconRSU* wsm)
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
    EV << wsm->getPos() << endl;
}


void ApplRSUBase::finish()
{
    BaseApplLayer::finish();
}


ApplRSUBase::~ApplRSUBase()
{

}


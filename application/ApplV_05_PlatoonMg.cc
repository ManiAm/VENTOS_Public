
#include "ApplV_05_PlatoonMg.h"

Define_Module(ApplVPlatoonMg);

void ApplVPlatoonMg::initialize(int stage)
{
    ApplV_AID::initialize(stage);

    if(mode != 4)
        return;

	if (stage == 0)
	{
	    maxPlnSize = par("maxPlatoonSize").longValue();
        optPlnSize = par("optPlatoonSize").longValue();
        mergeEnabled = par("mergeEnabled").boolValue();
        splitEnabled = par("splitEnabled").boolValue();

        vehicleState = state_idle;
        busy = false;

        // used in entry maneuver
        // ----------------------
        entryManeuverEvt = new cMessage("EntryEvt", KIND_TIMER);
        double offset = dblrand() * 10;
        scheduleAt(simTime() + 4., entryManeuverEvt); // todo: no offset for now!

        plnTIMER0 = new cMessage("listening to beacons", KIND_TIMER);

        leastDistFront = DBL_MAX;
        leastDistBack = DBL_MAX;

        // used in merge maneuver
        // ----------------------
        leadingPlnID = "";
        leadingPlnDepth = -1;

        plnTIMER1 = new cMessage("wait for merge reply", KIND_TIMER);
        plnTIMER1a = new cMessage("wait to catchup", KIND_TIMER);
        plnTIMER2 = new cMessage("wait for followers ack", KIND_TIMER);
        plnTIMER3 = new cMessage("wait for merge done", KIND_TIMER);
	}
}


void ApplVPlatoonMg::handleSelfMsg(cMessage* msg)
{
    // pass it down!
    ApplV_AID::handleSelfMsg(msg);

    entry_handleSelfMsg(msg);
    if(mergeEnabled) merge_handleSelfMsg(msg);
    if(splitEnabled) split_handleSelfMsg(msg);
}


void ApplVPlatoonMg::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down!
    ApplV_AID::onBeaconVehicle(wsm);

    // todo:
    if(simTime().dbl() >= 50)
    {
        optPlnSize = 13;
    }

    entry_BeaconFSM(wsm);
    if(mergeEnabled) merge_BeaconFSM(wsm);
    if(splitEnabled) split_BeaconFSM(wsm);
}


void ApplVPlatoonMg::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down!
    ApplV_AID::onBeaconRSU(wsm);
}


void ApplVPlatoonMg::onData(PlatoonMsg* wsm)
{
    // pass it down!
    ApplV_AID::onData(wsm);

    if(mergeEnabled) merge_DataFSM(wsm);
    if(splitEnabled) split_DataFSM(wsm);
}


// is called, every time the position of vehicle changes
void ApplVPlatoonMg::handlePositionUpdate(cObject* obj)
{
    // pass it down!
    ApplV_AID::handlePositionUpdate(obj);
}


PlatoonMsg*  ApplVPlatoonMg::prepareData(string receiver, uCommands type, string receivingPlatoonID, double dblValue, string strValue, deque<string> vecValue)
{
    if(mode != 4)
    {
        error("This application mode does not support platoon management!");
    }

    PlatoonMsg* wsm = new PlatoonMsg("platoonMsg");

    // add header length
    wsm->addBitLength(headerLength);

    // add payload length
    wsm->addBitLength(dataLengthBits);

    wsm->setWsmVersion(1);
    wsm->setSecurityType(1);

    if(dataOnSch)
    {
        wsm->setChannelNumber(Channels::SCH1);
    }
    else
    {
        wsm->setChannelNumber(Channels::CCH);
    }

    wsm->setDataRate(1);
    wsm->setPriority(dataPriority);
    wsm->setPsid(0);

    wsm->setSender(SUMOvID.c_str());
    wsm->setRecipient(receiver.c_str());
    wsm->setType(type);
    wsm->setSendingPlatoonID(plnID.c_str());
    wsm->setReceivingPlatoonID(receivingPlatoonID.c_str());
    wsm->setDblValue(dblValue);
    wsm->setStrValue(strValue.c_str());
    wsm->setQueueValue(vecValue);

    return wsm;
}


// print data message fields (for debugging purposes)
void ApplVPlatoonMg::printDataContent(PlatoonMsg* wsm)
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
    EV << wsm->getType() << " | ";
    EV << wsm->getSendingPlatoonID() << " | ";
    EV << wsm->getReceivingPlatoonID() << " | ";
    EV << wsm->getDblValue() << " | ";
    EV << wsm->getStrValue() << endl;
}


// upon changing myPlnDepth, this method should be called!
void ApplVPlatoonMg::updateColor()
{
    // non-platooned vehicle
    if(myPlnDepth == -1)
    {
        TraCIColor newColor = TraCIColor::fromTkColor("yellow");
        TraCI->getCommandInterface()->setColor(SUMOvID, newColor);
    }
    // platoon leader
    else if(myPlnDepth == 0)
    {
        TraCIColor newColor = TraCIColor::fromTkColor("red");
        TraCI->getCommandInterface()->setColor(SUMOvID, newColor);
    }
    // follower
    else if(myPlnDepth >= 1)
    {
        // todo
        TraCIColor newColor = TraCIColor::fromTkColor("blue");
        TraCI->getCommandInterface()->setColor(SUMOvID, newColor);
    }
}


const string ApplVPlatoonMg::stateToStr(int s)
{
    const char * statesStrings[] = {
        "state_idle",
        "state_platoonLeader", "state_platoonMember", "state_waitForLaneChange",
        "state_sendMergeReq", "state_waitForMergeReply", "state_mergeAccepted", "state_waitForCatchup",
        "state_sendMergeDone", "state_notifyFollowers",
        "state_state_waitForAllAcks", "state_sendMergeAccept",
        "state_waitForMergeDone", "state_mergeDone"
    };

    return statesStrings[s];
}


const string ApplVPlatoonMg::uCommandToStr(int c)
{
    const char * uCommandStrings[] = {
        "MERGE_REQ", "MERGE_ACCEPT",
        "MERGE_REJECT", "MERGE_DONE",
        "CHANGE_PL", "CHANGE_Tg",
        "SPLIT_REQ", "SPLIT_ACCEPT",
        "SPLIT_REJECT", "SPLIT_DONE",
        "LEAVE_REQ", "LEAVE_REJECT",
        "VOTE_LEADER", "ELECTED_LEADER",
        "DISSOLVE", "ACK"
    };

    return uCommandStrings[c];
}


void ApplVPlatoonMg::finish()
{
    ApplV_AID::finish();
}


ApplVPlatoonMg::~ApplVPlatoonMg()
{

}


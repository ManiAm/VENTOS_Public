
#include "ApplV_06_PlatoonMg.h"

Define_Module(ApplVPlatoonMg);


void ApplVPlatoonMg::initialize(int stage)
{
    ApplVPlatoonFormed::initialize(stage);

	if (stage == 0)
	{
	    if(plnMode != 3)
	        return;

	    maxPlnSize = par("maxPlatoonSize").longValue();
        optPlnSize = par("optPlatoonSize").longValue();
        mergeEnabled = par("mergeEnabled").boolValue();
        splitEnabled = par("splitEnabled").boolValue();

        vehicleState = state_idle;
        busy = false;
        mgrTIMER = new cMessage("manager", KIND_TIMER);
        scheduleAt(simTime() + 0.1, mgrTIMER);

        WATCH(vehicleState);
        WATCH(busy);

        // used in entry maneuver
        // ----------------------
        entryManeuverEvt = new cMessage("EntryEvt", KIND_TIMER);
        //double offset = dblrand() * 10;
        scheduleAt(simTime() + 4., entryManeuverEvt); // todo: no offset for now!

        plnTIMER0 = new cMessage("listening to beacons", KIND_TIMER);

        // used in merge maneuver
        // ----------------------
        leadingPlnID = "";
        leadingPlnDepth = -1;

        plnTIMER1  = new cMessage("wait for merge reply", KIND_TIMER);
        plnTIMER1a = new cMessage("wait to catchup", KIND_TIMER);
        plnTIMER2  = new cMessage("wait for followers ack", KIND_TIMER);
        plnTIMER3  = new cMessage("wait for merge done", KIND_TIMER);

        // used in split maneuver
        // ----------------------
        splittingVehicle = "";
        splittingDepth = -1;
        oldPlnID = "";

        plnTIMER4  = new cMessage("wait for split reply", KIND_TIMER);
        plnTIMER6  = new cMessage("wait for free agent ACK", KIND_TIMER);
        plnTIMER7  = new cMessage("wait for all ACKs", KIND_TIMER);
        plnTIMER5  = new cMessage("wait for change_pl", KIND_TIMER);
        plnTIMER8  = new cMessage("wait for split done", KIND_TIMER);
        plnTIMER8a  = new cMessage("wait for enough gap", KIND_TIMER);
	}
}


void ApplVPlatoonMg::handleSelfMsg(cMessage* msg)
{
    // pass it down!
    ApplVPlatoonFormed::handleSelfMsg(msg);

    if(plnMode != 3)
        return;

    if(msg == mgrTIMER) Coordinator();

    if(mergeEnabled) merge_handleSelfMsg(msg);
    if(splitEnabled) split_handleSelfMsg(msg);
    common_handleSelfMsg(msg);
    entry_handleSelfMsg(msg);
}


void ApplVPlatoonMg::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down!
    ApplVPlatoonFormed::onBeaconVehicle(wsm);

    if(plnMode != 3)
        return;

    if(mergeEnabled) merge_BeaconFSM(wsm);
    if(splitEnabled) split_BeaconFSM(wsm);
    common_BeaconFSM(wsm);
    entry_BeaconFSM(wsm);
}


void ApplVPlatoonMg::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down!
    ApplVPlatoonFormed::onBeaconRSU(wsm);
}


void ApplVPlatoonMg::onData(PlatoonMsg* wsm)
{
    // pass it down!
    ApplVPlatoonFormed::onData(wsm);

    if(plnMode != 3)
        return;

    if(mergeEnabled) merge_DataFSM(wsm);
    if(splitEnabled) split_DataFSM(wsm);
    common_DataFSM(wsm);
}


void ApplVPlatoonMg::Coordinator()
{
    if(simTime().dbl() >= 37)
    {
        optPlnSize = 13;
    }

    if(simTime().dbl() >= 77)
    {
        optPlnSize = 4;
    }

    if(simTime().dbl() >= 94)
    {
        optPlnSize = 10;
    }

    if(simTime().dbl() >= 131)
    {
        optPlnSize = 3;
    }
    if(simTime().dbl() >= 188)
    {
        optPlnSize = 2;
    }

    // check if we can split
    if(!busy && vehicleState == state_platoonLeader && plnSize > optPlnSize)
    {
        vehicleState = state_sendSplitReq;
        reportStateToStat();

        busy = true;

        split_DataFSM();
    }

    scheduleAt(simTime() + 0.1, mgrTIMER);
}


PlatoonMsg*  ApplVPlatoonMg::prepareData(string receiver, uCommands type, string receivingPlatoonID, double dblValue, string strValue, deque<string> vecValue)
{
    if(plnMode != 3)
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


// change follower blue color to show depth
// only platoon leader can call this!
void ApplVPlatoonMg::updateColorDepth()
{
    if(plnSize <= 0)
        error("plnSize is not right!");

    if(plnSize == 1)
        return;

    int offset = 255 / (plnSize-1);
    int *pickColor = new int[plnSize];
    pickColor[0] = -1;
    int count = 0;
    for(int i = 1; i < plnSize; i++)
    {
        pickColor[i] = count;
        count = count + offset;
    }

    // leader has all the followers in plnMembersList list
    for(unsigned int depth = 1; depth < plnMembersList.size(); depth++)
    {
        TraCIColor newColor = TraCIColor(pickColor[depth], pickColor[depth], 255, 255);
        TraCI->getCommandInterface()->setColor(plnMembersList[depth], newColor);
    }
}


void ApplVPlatoonMg::reportStateToStat()
{
    CurrentVehicleState *state = new CurrentVehicleState(SUMOvID.c_str(), stateToStr(vehicleState).c_str());
    simsignal_t Signal_VehicleState = registerSignal("VehicleState");
    nodePtr->emit(Signal_VehicleState, state);
}


const string ApplVPlatoonMg::stateToStr(int s)
{
    const char * statesStrings[] = {
        "state_idle", "state_platoonLeader", "state_platoonMember",

        "state_waitForLaneChange",

        "state_sendMergeReq", "state_waitForMergeReply", "state_mergeAccepted", "state_waitForCatchup",
        "state_sendMergeDone", "state_notifyFollowers",
        "state_state_waitForAllAcks", "state_sendMergeAccept",
        "state_waitForMergeDone", "state_mergeDone",

        "state_sendSplitReq", "state_waitForSplitReply", "state_makeItFreeAgent",
        "state_waitForAck", "state_splitDone", "state_changePL", "state_waitForAllAcks2",
        "state_waitForCHANGEPL", "state_sendingACK", "state_waitForSplitDone", "state_waitForGap",
    };

    return statesStrings[s];
}


void ApplVPlatoonMg::reportCommandToStat(PlatoonMsg* dataMsg)
{
    CurrentPlnMsg *plnMsg = new CurrentPlnMsg(dataMsg->getSender(), dataMsg->getRecipient(), uCommandToStr(dataMsg->getType()).c_str(), dataMsg->getSendingPlatoonID(), dataMsg->getReceivingPlatoonID());
    simsignal_t Signal_SentPlatoonMsg = registerSignal("SentPlatoonMsg");
    nodePtr->emit(Signal_SentPlatoonMsg, plnMsg);
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


// is called, every time the position of vehicle changes
void ApplVPlatoonMg::handlePositionUpdate(cObject* obj)
{
    // pass it down!
    ApplVPlatoonFormed::handlePositionUpdate(obj);
}


void ApplVPlatoonMg::finish()
{
    ApplVPlatoonFormed::finish();
}


ApplVPlatoonMg::~ApplVPlatoonMg()
{

}


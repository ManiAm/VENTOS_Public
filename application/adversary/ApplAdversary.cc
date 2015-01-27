
#include "ApplAdversary.h"

namespace VENTOS {

const simsignalwrap_t ApplAdversary::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

Define_Module(VENTOS::ApplAdversary);

ApplAdversary::~ApplAdversary()
{

}


void ApplAdversary::initialize(int stage)
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

        findHost()->subscribe(mobilityStateChangedSignal, this);

        // vehicle id in omnet++
		myId = getParentModule()->getIndex();

		myFullId = getParentModule()->getFullName();

		AttackT = par("AttackT").doubleValue();
		falsificationAttack = par("falsificationAttack").boolValue();
		replayAttack = par("replayAttack").boolValue();
		jammingAttack = par("jammingAttack").boolValue();

        JammingEvt = new cMessage("Jamming Event");

        if(jammingAttack)
	    {
            //scheduleAt(simTime(), JammingEvt);
	    }
	}
}


void ApplAdversary::finish()
{
    findHost()->unsubscribe(mobilityStateChangedSignal, this);
}


void ApplAdversary::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj)
{
    Enter_Method_Silent();

    if (signalID == mobilityStateChangedSignal)
    {
        ApplAdversary::handlePositionUpdate(obj);
    }
}


void ApplAdversary::handleSelfMsg(cMessage* msg)
{
    if(msg == JammingEvt)
    {
        if(simTime().dbl() >= AttackT)
        {
            DoJammingAttack();
        }

        // schedule for next jamming attack
        scheduleAt(simTime() + 0.001, JammingEvt);
    }
}


void ApplAdversary::handleLowerMsg(cMessage* msg)
{
    // Attack time has not arrived yet!
    if(simTime().dbl() < AttackT)
        return;

    // make sure msg is of type WaveShortMessage
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if ( string(wsm->getName()) == "beaconVehicle" )
    {
        BeaconVehicle* wsm = dynamic_cast<BeaconVehicle*>(msg);
        ASSERT(wsm);

        EV << "######### received a beacon!" << endl;

        if(falsificationAttack)
        {
            DoFalsificationAttack(wsm);
        }
        else if(replayAttack)
        {
            DoReplayAttack(wsm);
        }
    }
    else if( string(wsm->getName()) == "platoonMsg" )
    {
        PlatoonMsg* wsm = dynamic_cast<PlatoonMsg*>(msg);
        ASSERT(wsm);

        // ignore it!
    }

    delete msg;
}


void ApplAdversary::handlePositionUpdate(cObject* obj)
{
    ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
}


// adversary get a msg, modifies the acceleration and re-send it
void ApplAdversary::DoFalsificationAttack(BeaconVehicle* wsm)
{
    // duplicate the received beacon
    BeaconVehicle* FalseMsg = wsm->dup();

    // alter the acceleration field
    FalseMsg->setAccel(6.);

    // alter the position field
    //Coord *newCord = new Coord(0,0);
    //FalseMsg->setPos(*newCord);

    // send it
    sendDelayed(FalseMsg, 0., lowerLayerOut);

    EV << "## Altered msg is sent." << endl;
}


// adversary get a msg and re-send it with a delay (without altering the content)
void ApplAdversary::DoReplayAttack(BeaconVehicle * wsm)
{
    // duplicate the received beacon
    BeaconVehicle* FalseMsg = wsm->dup();

    // send it with delay
    double delay = 10;
    sendDelayed(FalseMsg, delay, lowerLayerOut);

    EV << "## Altered msg is sent with delay of " << delay << endl;
}


void ApplAdversary::DoJammingAttack()
{
    DummyMsg* dm = CreateDummyMessage();

    // send it
    sendDelayed(dm, 0, lowerLayerOut);
}


DummyMsg* ApplAdversary::CreateDummyMessage()
{
    DummyMsg* wsm = new DummyMsg("dummy");

    wsm->addBitLength(10000);

    wsm->setWsmVersion(1);
    wsm->setSecurityType(1);

    wsm->setChannelNumber(Channels::CCH);

    wsm->setDataRate(1);
    wsm->setPriority(2);
    wsm->setPsid(0);

    wsm->setPayload("dummy");

    return wsm;
}

}

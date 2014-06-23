
#include "ApplRSU.h"


MatrixXi ApplRSU::tableCount;
MatrixXd ApplRSU::tableProb;

Define_Module(ApplRSU);

void ApplRSU::initialize(int stage)
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

        // todo: change n and m dynamically!
        tableCount = MatrixXi::Zero(3, 2000);
        tableProb = MatrixXd::Constant(3, 2000, 0.1);
	}
	else if(stage==1)
	{

	}
}


void ApplRSU::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj)
{
    Enter_Method_Silent();

}


void ApplRSU::handleLowerMsg(cMessage* msg)
{
    // make sure msg is of type WaveShortMessage
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    // receive a lane change msg
    if(string(wsm->getName()) == "laneChangeMsg")
    {
        LaneChangeMsg* wsm = dynamic_cast<LaneChangeMsg*>(msg);
        ASSERT(wsm);

        ApplRSU::onLaneChange(wsm);
    }

    delete msg;
}


void ApplRSU::handleSelfMsg(cMessage* msg)
{
    if (msg == RSUBeaconEvt)
    {
        BeaconRSU* beaconMsg = prepareBeacon();

        EV << "## Created beacon msg for " << myFullId << endl;
        printBeaconContent(beaconMsg);

        // send it
        sendDelayedDown(beaconMsg, individualOffset);

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, RSUBeaconEvt);
    }
}


BeaconRSU* ApplRSU::prepareBeacon()
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
void ApplRSU::printBeaconContent(BeaconRSU* wsm)
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


void ApplRSU::onBeaconVehicle(BeaconVehicle* wsm)
{
    error("ApplVBeacon should not receive any beacon!");
}



void ApplRSU::onBeaconRSU(BeaconRSU* wsm)
{
    error("ApplVBeacon should not receive any beacon!");
}


void ApplRSU::onLaneChange(LaneChangeMsg* wsm)
{
    deque<string> input = wsm->getLaneChange();

    for(unsigned int i = 0; i < input.size(); i++)
    {
        // tokenize
        int readCount = 1;
        char_separator<char> sep("#", "", keep_empty_tokens);
        tokenizer< char_separator<char> > tokens(input[i], sep);

        string fromLane;
        string toLane;
        double fromX;
        double toX;
        double time;

        for(tokenizer< char_separator<char> >::iterator beg=tokens.begin(); beg!=tokens.end();++beg)
        {
            if(readCount == 1)
            {
                fromLane = (*beg);
               //EV << "token 1: " << fromLane << endl;
            }
            else if(readCount == 2)
            {
                toLane = (*beg);
               //EV << "token 2: " << toLane << endl;
            }
            else if(readCount == 3)
            {
                fromX = atof( (*beg).c_str() );
               //EV << "token 3: " << x << endl;
            }
            else if(readCount == 4)
            {
                toX = atof( (*beg).c_str() );
                //EV << "token 4: " << y << endl;
            }
            else if(readCount == 5)
            {
                time = atof( (*beg).c_str() );
               //EV << "token 5: " << time << endl;
            }

            readCount++;
        }

        // todo: change them dynamically
        int index_N_start = floor(fromX / 5);
        int index_N_end = floor(toX / 5);
        int index_M = -1;

        if(fromLane == "")
            fromLane = toLane;

        if(fromLane == "1to2_0")
        {
            index_M = 0;
        }
        else if(fromLane == "1to2_1")
        {
            index_M = 1;
        }
        else if(fromLane == "1to2_2")
        {
            index_M = 2;
        }

        // increase all corresponding indices in tableCount by 1
        for(int j = index_N_start; j <= index_N_end; j++)
        {
            tableCount(index_M, j) = tableCount(index_M, j) + 1;
        }
    }
}


void ApplRSU::finish()
{

}


ApplRSU::~ApplRSU()
{

}


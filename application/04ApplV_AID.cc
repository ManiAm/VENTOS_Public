
#include "04ApplV_AID.h"


Define_Module(ApplV_AID);

void ApplV_AID::initialize(int stage)
{
    ApplVBeacon::initialize(stage);

	if (stage == 0)
	{
        AID = par("AID").boolValue();

        // NED variables (data messages)
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        fromLane = "";
        toLane = "";
        fromX = 0;
        toX = 0;

        laneChanges.clear();
	}
}


// handle my own SelfMsg
void ApplV_AID::handleSelfMsg(cMessage* msg)
{
    ApplVBeacon::handleSelfMsg(msg);
}


void ApplV_AID::onBeaconVehicle(BeaconVehicle* wsm)
{

}


void ApplV_AID::onBeaconRSU(BeaconRSU* wsm)
{
    // I received a beacon from RSU
    // send my laneChanges
    if( !laneChanges.empty() )
    {
        LaneChangeMsg* dataMsg = ApplV_AID::prepareData(wsm->getSender(), laneChanges);
        ApplV_AID::printDataContent(dataMsg);
        sendDelayedDown(dataMsg, individualOffset);
        EV << "### " << SUMOvID << ": sent ClaneChangeMsg message." << endl;

        laneChanges.clear();
       // lastLaneName = manager->commandGetLaneId(SUMOvID);
    }
}


void ApplV_AID::onData(PlatoonMsg* wsm)
{

}


LaneChangeMsg*  ApplV_AID::prepareData(std::string receiver, std::deque<std::string> vecValue)
{
    LaneChangeMsg* wsm = new LaneChangeMsg("laneChangeMsg");

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
    wsm->setLaneChange(vecValue);

    return wsm;
}


// print data message fields (for debugging purposes)
void ApplV_AID::printDataContent(LaneChangeMsg* wsm)
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

    std::deque<std::string> tmp = wsm->getLaneChange();

    for(unsigned int i = 0; i< tmp.size(); i++)
    {
        EV << tmp[i] << endl;
    }

    EV << endl;

}


// is called, every time the position of vehicle changes
void ApplV_AID::handlePositionUpdate(cObject* obj)
{
    ApplVBeacon::handlePositionUpdate(obj);

    toLane = manager->commandGetLaneId(SUMOvID);

    // if we change lane
    if(fromLane != toLane)
    {
        std::ostringstream str;
        toX =  ( manager->commandGetVehiclePos(SUMOvID) ).x;
        str << fromLane <<  "#" << toLane << "#" << fromX << "#" << toX << "#" << simTime().dbl();
        laneChanges.push_back( str.str() );

        fromLane = toLane;
        fromX = toX;
    }
}


void ApplV_AID::finish()
{
    ApplVBeacon::finish();
}


ApplV_AID::~ApplV_AID()
{

}


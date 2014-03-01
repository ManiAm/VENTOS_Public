
#include "ApplVPlatoon.h"

Define_Module(ApplVPlatoon);

void ApplVPlatoon::initialize(int stage)
{
    ApplVBeaconPlatoonLeader::initialize(stage);

	if (stage == 0)
	{
	    // NED variables (platoon formation)
        platoonFormation = par("platoonFormation").boolValue();
        maxPlatoonSize = par("maxPlatoonSize").longValue();

        // NED variables (platoon messages)
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        sentMessage = false;
        lastDroveAt = simTime();
        vehicleState = state_idle;
	}
}


void ApplVPlatoon::handleLowerMsg(cMessage* msg)
{
    // make sure msg is of type WaveShortMessage
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()) == "beacon")
    {
        if(platoonFormation)
        {
            ApplVPlatoon::onBeacon(wsm);
        }
        else
        {
            ApplVBeaconPlatoonLeader::handleLowerMsg(msg);
        }
    }
    else if (std::string(wsm->getName()) == "data")
    {
        if(platoonFormation)
        {
            ApplVPlatoon::onData(wsm);
        }
    }
    else
    {
        error("Unknown message received in ApplVPlatoon!");
    }
}


void ApplVPlatoon::handleSelfMsg(cMessage* msg)
{
    ApplVBeaconPlatoonLeader::handleSelfMsg(msg);
}


void ApplVPlatoon::onBeacon(WaveShortMessage* wsm)
{
    if(!platoonFormation)
    {
        error("platoonFormation is off in ApplVPlatoon::onBeacon");
    }



}


void ApplVPlatoon::onData(WaveShortMessage* wsm)
{
    if(!platoonFormation)
    {
        error("platoonFormation is off in ApplVPlatoon::onData");
    }





    findHost()->getDisplayString().updateWith("r=16,green");
    annotations->scheduleErase(1, annotations->drawLine(wsm->getPos(), traci->getPositionAt(simTime()), "blue"));

    if (traci->getRoadId()[0] != ':') traci->commandChangeRoute(wsm->getWsmData(), 9999);
    if (!sentMessage) sendMessage(wsm->getWsmData());


    /*
    DBG << "Received beacon priority  " << wsm->getPriority() << " at " << simTime() << std::endl;
    int senderId = wsm->getSenderAddress();

    if (sendData)
    {
        t_channel channel = dataOnSch ? type_SCH : type_CCH;
        sendWSM(prepareWSM("data", dataLengthBits, channel, dataPriority, senderId,2));
    }
    */
}


void ApplVPlatoon::sendMessage(std::string blockedRoadId)
{
    sentMessage = true;

    // t_channel channel = dataOnSch ? type_SCH : type_CCH;
    // WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
    // wsm->setWsmData(blockedRoadId.c_str());
    // sendWSM(wsm);
}


// is called, every time the position of vehicle changes
void ApplVPlatoon::handlePositionUpdate(cObject* obj)
{
    ApplVBeacon::handlePositionUpdate(obj);

    // stopped for at least 10s?
    if (traci->getSpeed() < 1)
    {
        if (simTime() - lastDroveAt >= 10)
        {
            findHost()->getDisplayString().updateWith("r=16,red");
            // if (!sentMessage) sendMessage( traci->getRoadId() );
        }
    }
    else
    {
        lastDroveAt = simTime();
    }
}


void ApplVPlatoon::FSMchangeState()
{
    if(vehicleState == state_idle)
    {
        // check for leading vehicle
        std::string vleaderID = manager->commandGetLeading_M(SUMOvID);

        if(vleaderID == "")
        {
            EV << "This vehicle has no leading vehicle." << std::endl;
        }


    }
    else if(true)
    {

    }


}


void ApplVPlatoon::finish()
{

}


ApplVPlatoon::~ApplVPlatoon()
{

}


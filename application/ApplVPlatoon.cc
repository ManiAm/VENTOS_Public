
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

        TIMER1 = new cMessage("timer_wait_for_beacon_from_leading", timer_wait_for_beacon_from_leading);
        TIMER2 = new cMessage("timer_wait_for_JOIN_response", timer_wait_for_JOIN_response);
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
            Beacon* wsm = dynamic_cast<Beacon*>(msg);
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
            PlatoonMsg* wsm = dynamic_cast<PlatoonMsg*>(msg);
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

    if (msg == TIMER1)
    {
        if(vehicleState == state_wait_for_beacon)
        {
            vehicleState = stateT_create_new_platoon;
            FSMchangeState();
            return;
        }
    }
    else if (msg == TIMER2)
    {
        if(vehicleState == state_ask_to_join)
        {
            vehicleState = stateT_create_new_platoon;
            FSMchangeState();
            return;
        }
    }
}


void ApplVPlatoon::onBeacon(Beacon* wsm)
{
    if(!platoonFormation)
    {
        error("platoonFormation is off in ApplVPlatoon::onBeacon");
    }

    if( vehicleState == state_wait_for_beacon && TIMER1->isScheduled() )
    {
        vehicleState = state_ask_to_join;
        cancelEvent(TIMER1);
        myLeadingBeacon = wsm->dup();  // store a copy of beacon for future use
        // todo: send JOIN request
        scheduleAt(simTime() + 1., TIMER2);
    }
}


void ApplVPlatoon::onData(PlatoonMsg* wsm)
{
    if(!platoonFormation)
    {
        error("platoonFormation is off in ApplVPlatoon::onData");
    }

    // todo:
    if (wsm == NULL /*JOIN_REJECT*/)
    {
        if(vehicleState == state_ask_to_join)
        {
            vehicleState = stateT_create_new_platoon;
            cancelEvent(TIMER2);
            FSMchangeState();
            return;
        }
    }
    // todo:
    else if (wsm == NULL /*JOIN_ACCEPT*/)
    {
        if(vehicleState == state_ask_to_join)
        {
            vehicleState = stateT_joining;
            FSMchangeState();
            return;
        }
    }
    // todo:
    else if(wsm == NULL /*JOIN_request*/)
    {
        if(vehicleState == state_platoonLeader)
        {
            if(platoonSize == maxPlatoonSize)
            {
                // todo: send JOIN_REJECT
            }
            else
            {
                // todo: send JOIN_ACCEPT
                ++platoonSize;
                // todo: send CHANGE_Tg
            }
        }
    }




    findHost()->getDisplayString().updateWith("r=16,green");
  //  annotations->scheduleErase(1, annotations->drawLine(wsm->getPos(), traci->getPositionAt(simTime()), "blue"));

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
    // #######################################################################
    if(vehicleState == state_idle)
    {
        // check for leading vehicle
        std::string vleaderID = manager->commandGetLeading_M(SUMOvID);

        if(vleaderID == "")
        {
            EV << "This vehicle has no leading vehicle." << std::endl;
            vehicleState = stateT_create_new_platoon;
            FSMchangeState();
            return;
        }
        else
        {
            vehicleState = state_wait_for_beacon;
            FSMchangeState();
            return;
        }
    }
    // #######################################################################
    else if(vehicleState == state_wait_for_beacon)
    {
        // waiting for a beacon from leading vehicle
        scheduleAt(simTime() + 1., TIMER1);
        return;
    }
    // #######################################################################
    else if(vehicleState == stateT_create_new_platoon)
    {
        manager->commandSetTg(SUMOvID, 3.5);
        platoonID = SUMOvID;
        myPlatoonDepth = 0;
        platoonSize = 1;

        vehicleState = state_platoonLeader;
        FSMchangeState();
        return;
    }
    // #######################################################################
    else if(vehicleState == stateT_joining)
    {
        cancelEvent(TIMER2);
        manager->commandSetSpeed(SUMOvID, 30.);
        manager->commandSetTg(SUMOvID, 0.55);
        platoonID = myLeadingBeacon->getPlatoonID();
        myPlatoonDepth = myLeadingBeacon->getPlatoonDepth() + 1;

        vehicleState = state_platoonMember;
        FSMchangeState();
        return;
    }
}


void ApplVPlatoon::finish()
{

}


ApplVPlatoon::~ApplVPlatoon()
{

}

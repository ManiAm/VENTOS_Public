
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

        FSMchangeState();
	}
}


void ApplVPlatoon::handleLowerMsg(cMessage* msg)
{
    /*
    uint8_t* color = manager->commandGetVehicleColor(SUMOvID);

    uint8_t R = color[0];
    uint8_t G = color[1];
    uint8_t B = color[2];
    uint8_t A = color[3];

    manager->commandSetVehicleColor(SUMOvID, 255, 255, 0, 100);
    */


    // make sure msg is of type WaveShortMessage
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()) == "beacon")
    {
        if(platoonFormation)
        {
            Beacon* wsm = dynamic_cast<Beacon*>(msg);
            ASSERT(wsm);

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
            ASSERT(wsm);

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

        // send JOIN request
        PlatoonMsg* dataMsg = prepareData(wsm->getSender(), JOIN_request, wsm->getPlatoonID(), -1);
        EV << "## Created JOIN request in vehicle: " << SUMOvID << std::endl;
        printDataContent(dataMsg);
        sendDelayedDown(dataMsg,individualOffset);

        scheduleAt(simTime() + 1., TIMER2);
    }
}


void ApplVPlatoon::onData(PlatoonMsg* wsm)
{
    if(!platoonFormation)
    {
        error("platoonFormation is off in ApplVPlatoon::onData");
    }

    if (wsm->getReq_res_type() == JOIN_REJECT_response)
    {
        if(vehicleState == state_ask_to_join)
        {
            vehicleState = stateT_create_new_platoon;
            cancelEvent(TIMER2);
            FSMchangeState();
            return;
        }
    }
    else if (wsm->getReq_res_type() == JOIN_ACCEPT_response)
    {
        if(vehicleState == state_ask_to_join)
        {
            vehicleState = stateT_joining;
            FSMchangeState();
            return;
        }
    }
    else if(wsm->getReq_res_type() == JOIN_request)
    {
        if(vehicleState == state_platoonLeader)
        {
            if(platoonSize == maxPlatoonSize)
            {
                // send JOIN_REJECT
                PlatoonMsg* dataMsg = prepareData(wsm->getSender(), JOIN_REJECT_response, wsm->getSendingPlatoonID(), -1);
                EV << "## Created JOIN_REJECT response in vehicle: " << SUMOvID << std::endl;
                printDataContent(dataMsg);
                sendDelayedDown(dataMsg,individualOffset);
            }
            else
            {
                // send JOIN_ACCEPT
                PlatoonMsg* dataMsg1 = prepareData(wsm->getSender(), JOIN_ACCEPT_response, wsm->getSendingPlatoonID(), -1);
                EV << "## Created JOIN_ACCEPT response in vehicle: " << SUMOvID << std::endl;
                printDataContent(dataMsg1);
                sendDelayedDown(dataMsg1,individualOffset);

                ++platoonSize;

                // send CHANGE_Tg
                // todo: make it dynamic
                double newTg = 0.55;
                PlatoonMsg* dataMsg2 = prepareData("broadcast", CHANGE_Tg, platoonID, newTg);
                EV << "## Created CHANGE_Tg  message in vehicle: " << SUMOvID << std::endl;
                printDataContent(dataMsg2);
                sendDelayedDown(dataMsg2,individualOffset);
            }
        }
    }
    else if(wsm->getReq_res_type() == CHANGE_Tg)
    {
        // check if this is coming from my platoon leader
        if( std::string(wsm->getReceivingPlatoonID()) == platoonID )
        {
            manager->commandSetTg(SUMOvID, wsm->getValue());
        }
    }






   // findHost()->getDisplayString().updateWith("r=16,green");

  //  annotations->scheduleErase(1, annotations->drawLine(wsm->getPos(), traci->getPositionAt(simTime()), "blue"));

  //  if (traci->getRoadId()[0] != ':') traci->commandChangeRoute(wsm->getWsmData(), 9999);
  //  if (!sentMessage) sendMessage(wsm->getWsmData());


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
           // findHost()->getDisplayString().updateWith("r=16,red");
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
    EV << "### " << SUMOvID << " : current status is : " << vehicleState << endl;

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

        nodePtr->getDisplayString().updateWith("i2=status/checkmark,green");

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


PlatoonMsg*  ApplVPlatoon::prepareData(std::string receiver, int type, std::string receivingPlatoonID, int value)
{
    if(!platoonFormation)
    {
        error("platoonFormation is off in ApplVPlatoon::prepareData");
    }

    PlatoonMsg* wsm = new PlatoonMsg("data");

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
    wsm->setReq_res_type(type);
    wsm->setSendingPlatoonID(platoonID.c_str());
    wsm->setReceivingPlatoonID(receivingPlatoonID.c_str());
    wsm->setValue(value);

    return wsm;
}


// print data message fields (for debugging purposes)
void ApplVPlatoon::printDataContent(PlatoonMsg* wsm)
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
    EV << wsm->getReq_res_type() << " | ";
    EV << wsm->getSendingPlatoonID() << " | ";
    EV << wsm->getReceivingPlatoonID() << " | ";
    EV << wsm->getValue() << std::endl;
}


void ApplVPlatoon::finish()
{

}


ApplVPlatoon::~ApplVPlatoon()
{

}

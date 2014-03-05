
#include "ApplVPlatoonFormation.h"

Define_Module(ApplVPlatoonFormation);

void ApplVPlatoonFormation::initialize(int stage)
{
    ApplVPlatoon::initialize(stage);

	if (stage == 0)
	{
	    // NED variables (platoon formation)
        platoonFormation = par("platoonFormation").boolValue();
        maxPlatoonSize = par("maxPlatoonSize").longValue();
        timer1Value = par("timer1Value").doubleValue();
        timer2Value = par("timer2Value").doubleValue();

        // NED variables (platoon messages)
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        vehicleState = state_idle;

        TIMER1 = new cMessage("timer_wait_for_beacon_from_leading", timer_wait_for_beacon_from_leading);
        TIMER2 = new cMessage("timer_wait_for_JOIN_response", timer_wait_for_JOIN_response);

        int offset = 255 / (maxPlatoonSize-2);
        pickColor = new int[maxPlatoonSize];
        pickColor[0] = -1;
        int count = 0;
        for(int i = 1; i < maxPlatoonSize; i++)
        {
            pickColor[i] = count;
            count = count + offset;
        }

        if(platoonFormation)
            FSMchangeState();
	}
}


void ApplVPlatoonFormation::handleLowerMsg(cMessage* msg)
{
    error("ApplVPlatoonFormation should not receive any lower message!");
}


void ApplVPlatoonFormation::handleSelfMsg(cMessage* msg)
{
    if(!platoonFormation)
    {
        ApplVPlatoon::handleSelfMsg(msg);
        return;
    }

    if (msg->getKind() == SEND_BEACON_EVT)
    {
        Beacon* Msg = ApplVBeacon::prepareBeacon();

        // fill-in the related fields to platoon
        Msg->setPlatoonID(platoonID.c_str());
        Msg->setPlatoonDepth(myPlatoonDepth);

        EV << "## Created beacon msg for vehicle: " << SUMOvID << std::endl;
        ApplVBeacon::printBeaconContent(Msg);

        // send it
        sendDelayedDown(Msg,individualOffset);

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, sendBeaconEvt);
    }
    else if (msg == TIMER1)
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


void ApplVPlatoonFormation::onBeacon(Beacon* wsm)
{
    if(!platoonFormation)
    {
        ApplVPlatoon::onBeacon(wsm);
        return;
    }

    // waiting for a beacon from our leading vehicle
    if( vehicleState == state_wait_for_beacon && TIMER1->isScheduled() && isBeaconFromLeading(wsm) )
    {
        vehicleState = state_ask_to_join;
        cancelEvent(TIMER1);
        myLeadingBeacon = wsm->dup();  // store a copy of beacon for future use

        // send JOIN request
        PlatoonMsg* dataMsg = prepareData(wsm->getSender(), JOIN_request, wsm->getPlatoonID(), -1);
        printDataContent(dataMsg);
        sendDelayedDown(dataMsg,individualOffset);
        EV << "### " << SUMOvID << ": sent JOIN request." << endl;

        scheduleAt(simTime() + timer2Value, TIMER2);
    }
}


void ApplVPlatoonFormation::onData(PlatoonMsg* wsm)
{
    if(!platoonFormation)
        return;

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
                printDataContent(dataMsg);
                sendDelayedDown(dataMsg,individualOffset);
                EV << "### " << SUMOvID << ": sent JOIN_REJECT." << endl;
            }
            else
            {
                // send JOIN_ACCEPT
                PlatoonMsg* dataMsg1 = prepareData(wsm->getSender(), JOIN_ACCEPT_response, wsm->getSendingPlatoonID(), -1);
                printDataContent(dataMsg1);
                sendDelayedDown(dataMsg1,individualOffset);
                EV << "### " << SUMOvID << ": sent JOIN_ACCEPT." << endl;

                ++platoonSize;

                // send CHANGE_Tg
                // todo: make it dynamic
                double newTg = 0.55;
                PlatoonMsg* dataMsg2 = prepareData("broadcast", CHANGE_Tg, platoonID, newTg);
                printDataContent(dataMsg2);
                sendDelayedDown(dataMsg2,individualOffset);
                EV << "### " << SUMOvID << ": sent CHANGE_Tg with value " << newTg << endl;
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
}


// is called, every time the position of vehicle changes
void ApplVPlatoonFormation::handlePositionUpdate(cObject* obj)
{
    ApplVPlatoon::handlePositionUpdate(obj);
}


void ApplVPlatoonFormation::FSMchangeState()
{
    if(vehicleState == state_idle)
    {
        EV << "### " << SUMOvID << ": current vehicle status is idle." << endl;

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
        EV << "### " << SUMOvID << ": current vehicle status is wait_for_beacon." << endl;

        // waiting for a beacon from leading vehicle
        scheduleAt(simTime() + timer1Value, TIMER1);
        return;
    }
    // #######################################################################
    else if(vehicleState == stateT_create_new_platoon)
    {
        EV << "### " << SUMOvID << ": current vehicle status is create-new_platoon." << endl;

        manager->commandSetTg(SUMOvID, 3.5);
        platoonID = SUMOvID;
        myPlatoonDepth = 0;
        platoonSize = 1;

        // nodePtr->getDisplayString().updateWith("i2=status/checkmark,green");
        TraCIColor newColor = TraCIColor::fromTkColor("red");
        manager->commandSetVehicleColor(SUMOvID, newColor);

        vehicleState = state_platoonLeader;
        FSMchangeState();
        return;
    }
    // #######################################################################
    else if(vehicleState == stateT_joining)
    {
        EV << "### " << SUMOvID << ": current vehicle status is joining." << endl;

        cancelEvent(TIMER2);
        manager->commandSetSpeed(SUMOvID, 30.);
        manager->commandSetTg(SUMOvID, 0.55);
        platoonID = myLeadingBeacon->getPlatoonID();
        myPlatoonDepth = myLeadingBeacon->getPlatoonDepth() + 1;

        if(SUMOvID == "CACC3")
        {
            EV << "hello";
        }


        TraCIColor newColor = TraCIColor(pickColor[myPlatoonDepth], pickColor[myPlatoonDepth], 255, 255);
        manager->commandSetVehicleColor(SUMOvID, newColor);

        vehicleState = state_platoonMember;
        FSMchangeState();
        return;
    }
    else if(vehicleState == state_platoonLeader)
    {
        EV << "### " << SUMOvID << ": current vehicle status is platoonLeader with depth " << myPlatoonDepth << endl;
    }
    else if(vehicleState == state_platoonMember)
    {
        EV << "### " << SUMOvID << ": current vehicle status is platoonMember with depth " << myPlatoonDepth << endl;
    }
}


PlatoonMsg*  ApplVPlatoonFormation::prepareData(std::string receiver, int type, std::string receivingPlatoonID, int value)
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
void ApplVPlatoonFormation::printDataContent(PlatoonMsg* wsm)
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


// get the gap to the leading vehicle using sonar
double ApplVPlatoonFormation::getGap(std::string vleaderID)
{
    if(vleaderID == "")
        return -1;

    double gap = manager->commandGetLanePosition(vleaderID) - manager->commandGetLanePosition(SUMOvID) - manager->commandGetVehicleLength(vleaderID);

    return gap;
}


bool ApplVPlatoonFormation::isBeaconFromLeading(Beacon* wsm)
{
    // step 1: check if a leading vehicle is present

    std::string vleaderID = manager->commandGetLeading_M(SUMOvID);

    if(vleaderID == "")
    {
        EV << "This vehicle has no leading vehicle." << std::endl;
        return false;
    }

    // step 2: is it on the same lane?

    std::string myLane = manager->commandGetLaneId(SUMOvID);
    std::string beaconLane = wsm->getLane();

    EV << "I am on lane " << manager->commandGetLaneId(SUMOvID) << ", and other vehicle is on lane " << wsm->getLane() << std::endl;

    if( myLane != beaconLane )
    {
        EV << "Not on the same lane!" << std::endl;
        return false;
    }

    EV << "We are on the same lane!" << std::endl;

    // step 3: is the distance equal to gap?

    Coord cord = manager->commandGetVehiclePos(SUMOvID);
    double dist = sqrt( pow(cord.x - wsm->getPos().x, 2) +  pow(cord.y - wsm->getPos().y, 2) );

    // subtract the length of the leading vehicle from dist
    dist = dist - manager->commandGetVehicleLength(vleaderID);

    // use our sonar to compute the gap
    double gap = getGap(vleaderID);

    EV << "my coord (x,y): " << cord.x << "," << cord.y << std::endl;
    EV << "other coord (x,y): " << wsm->getPos().x << "," << wsm->getPos().y << std::endl;
    EV << "distance is " << dist << ", and gap is " << gap << std::endl;

    double diff = fabs(dist - gap);

    if(diff > 0.001)
    {
        EV << "distance does not match the gap!" << std::endl;
        return false;
    }

    EV << "This beacon is from the leading vehicle!" << std::endl;
    return true;
}


void ApplVPlatoonFormation::finish()
{

}


ApplVPlatoonFormation::~ApplVPlatoonFormation()
{

}

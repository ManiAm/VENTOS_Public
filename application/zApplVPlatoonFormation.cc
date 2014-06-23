
#include "zApplVPlatoonFormation.h"

Define_Module(ApplVPlatoonFormation);

void ApplVPlatoonFormation::initialize(int stage)
{
    // ApplVPlatoon::initialize(stage);

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

        // only if platoonFormation is on
        // otherwise, ApplVPlatoon will initialize these accordingly
        if( platoonFormation )
        {
            myPlatoonDepth = -1;
            platoonID = "";
            platoonSize = -1;
        }

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
        // ApplVPlatoon::handleSelfMsg(msg);
        return;
    }

    if (msg == VehicleBeaconEvt)
    {
        BeaconVehicle* Msg = ApplVBeacon::prepareBeacon();

        // fill-in the related fields to platoon
        Msg->setPlatoonID(platoonID.c_str());
        Msg->setPlatoonDepth(myPlatoonDepth);

        EV << "## Created beacon msg for vehicle: " << SUMOvID << endl;
        ApplVBeacon::printBeaconContent(Msg);

        // send it
        sendDelayedDown(Msg,individualOffset);

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, VehicleBeaconEvt);
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


void ApplVPlatoonFormation::onBeaconVehicle(BeaconVehicle* wsm)
{
    if(!platoonFormation)
    {
        // ApplVPlatoon::onBeaconVehicle(wsm);
        return;
    }

    // waiting for a beacon from our leading vehicle
    if( vehicleState == state_wait_for_beacon && TIMER1->isScheduled() /* && ApplVPlatoon::isBeaconFromLeading(wsm)*/ )
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
                queue.push_back( wsm->getSender() );

                // send CHANGE_Tg
                if( platoonSize > floor(maxPlatoonSize / 2) )
                {
                    PlatoonMsg* dataMsg2 = prepareData("broadcast", CHANGE_Tg, platoonID, 0.6);
                    printDataContent(dataMsg2);
                    sendDelayedDown(dataMsg2,individualOffset);
                    EV << "### " << SUMOvID << ": sent CHANGE_Tg with value " << 0.6 << endl;
                }
            }
        }
    }
    else if(wsm->getReq_res_type() == CHANGE_Tg)
    {
        // check if this is coming from my platoon leader
        if( string(wsm->getReceivingPlatoonID()) == platoonID )
        {
            TraCI->commandSetTg(SUMOvID, wsm->getDblValue());
        }
    }
}


// is called, every time the position of vehicle changes
void ApplVPlatoonFormation::handlePositionUpdate(cObject* obj)
{
    // ApplVPlatoon::handlePositionUpdate(obj);
}


void ApplVPlatoonFormation::FSMchangeState()
{
    if(vehicleState == state_idle)
    {
        EV << "### " << SUMOvID << ": current vehicle status is idle." << endl;

        // check for leading vehicle
        vector<string> res = TraCI->commandGetLeading(SUMOvID, sonarDist);
        string vleaderID = res[0];

        if(vleaderID == "")
        {
            EV << "This vehicle has no leading vehicle." << endl;
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

        TraCI->commandSetTg(SUMOvID, 3.5);
        platoonID = SUMOvID;
        myPlatoonDepth = 0;
        platoonSize = 1;
        queue.clear();

        // nodePtr->getDisplayString().updateWith("i2=status/checkmark,green");
        TraCIColor newColor = TraCIColor::fromTkColor("red");
        TraCI->commandSetVehicleColor(SUMOvID, newColor);

        vehicleState = state_platoonLeader;
        FSMchangeState();
        return;
    }
    // #######################################################################
    else if(vehicleState == stateT_joining)
    {
        EV << "### " << SUMOvID << ": current vehicle status is joining." << endl;

        cancelEvent(TIMER2);
        TraCI->getCommandInterface()->setSpeed(SUMOvID, 30.);
        TraCI->commandSetTg(SUMOvID, 0.55);
        platoonID = myLeadingBeacon->getPlatoonID();
        myPlatoonDepth = myLeadingBeacon->getPlatoonDepth() + 1;

        TraCIColor newColor = TraCIColor(pickColor[myPlatoonDepth], pickColor[myPlatoonDepth], 255, 255);
        TraCI->commandSetVehicleColor(SUMOvID, newColor);

        vehicleState = state_platoonMember;
        FSMchangeState();
        return;
    }
    else if(vehicleState == state_platoonLeader)
    {
        EV << "### " << SUMOvID << ": current vehicle status is platoonLeader." << endl;
    }
    else if(vehicleState == state_platoonMember)
    {
        EV << "### " << SUMOvID << ": current vehicle status is platoonMember." << endl;
    }
}


PlatoonMsg*  ApplVPlatoonFormation::prepareData(string receiver, int type, string receivingPlatoonID, double dblValue, string strValue, deque<string> vecValue)
{
    if(!platoonFormation)
    {
        error("platoonFormation is off in ApplVPlatoon::prepareData");
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
    wsm->setReq_res_type(type);
    wsm->setSendingPlatoonID(platoonID.c_str());
    wsm->setReceivingPlatoonID(receivingPlatoonID.c_str());
    wsm->setDblValue(dblValue);
    wsm->setStrValue(strValue.c_str());
    wsm->setQueueValue(vecValue);

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
    EV << wsm->getDblValue() << " | ";
    EV << wsm->getStrValue() << endl;
}


void ApplVPlatoonFormation::finish()
{
    // ApplVPlatoon::finish();
}


ApplVPlatoonFormation::~ApplVPlatoonFormation()
{

}

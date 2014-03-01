
#include "ApplVBeaconPlatoonLeader.h"

Define_Module(ApplVBeaconPlatoonLeader);

void ApplVBeaconPlatoonLeader::initialize(int stage)
{
    ApplVBeacon::initialize(stage);

	if (stage == 0)
	{
        one_vehicle_look_ahead = par("one_vehicle_look_ahead");
        platoonLeaderID = par("platoonLeaderID").stringValue();
        platoonID = par("platoonID").longValue();

        if(SUMOvID == platoonLeaderID)
        {
            myPlatoonDepth = 0;
            // platoonID       : got the value from above.
            // platoonLeaderID : got the value from above.
        }
        else
        {
            myPlatoonDepth = -1;
            platoonID = -1;         // we are not part of a platoon (yet!).
            platoonLeaderID = "";   // we do not know the platoon leader (yet!).
        }
	}
}


void ApplVBeaconPlatoonLeader::handleLowerMsg(cMessage* msg)
{
    if(one_vehicle_look_ahead)
    {
        ApplVBeacon::handleLowerMsg(msg);
        return;
    }

    // make sure msg is of type WaveShortMessage
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()) == "beacon")
    {
        // vehicles other than CACC should ignore the received beacon
        if( isCACC() )
        {
            if( !ApplVBeacon::dropBeacon(droppT, droppV, plr) )
            {
                ApplVBeaconPlatoonLeader::onBeacon(wsm);
            }
            // drop the beacon, and report it to statistics
            else
            {
                bool result = isBeaconFromPlatoonLeader(wsm);

                if(result)
                {
                    // a beacon from preceding vehicle is drooped
                    simsignal_t Signal_beaconD = registerSignal("beaconD");
                    nodePtr->emit(Signal_beaconD, 1);
                }
                else
                {
                    // a beacon from other vehicle is drooped
                    simsignal_t Signal_beaconD = registerSignal("beaconD");
                    nodePtr->emit(Signal_beaconD, 2);
                }
            }
        }
    }
    else
    {
        error("Unknown message received in ApplVBeaconPlatoonLeader!");
    }
}


void ApplVBeaconPlatoonLeader::handleSelfMsg(cMessage* msg)
{
    if(one_vehicle_look_ahead)
    {
        ApplVBeacon::handleSelfMsg(msg);
        return;
    }

    if (msg->getKind() == SEND_BEACON_EVT)
    {
        WaveShortMessage* Msg = ApplVBeacon::prepareBeacon("beacon", beaconLengthBits, type_CCH, beaconPriority, 0);

        // fill-in the fields related to platoon
        WaveShortMessage* beaconMsg = fillBeaconPlatoon(Msg);

        EV << "## Created beacon msg for vehicle: " << SUMOvID << std::endl;
        ApplVBeacon::printBeaconContent(beaconMsg);

        // send it
        sendDelayedDown(beaconMsg,individualOffset);

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, sendBeaconEvt);
    }
}


WaveShortMessage* ApplVBeaconPlatoonLeader::fillBeaconPlatoon(WaveShortMessage *wsm)
{
    wsm->setPlatoonID(platoonID);
    wsm->setPlatoonDepth(myPlatoonDepth);

    return wsm;
}


// for "one_vehicle_look_ahead = false" case only
void ApplVBeaconPlatoonLeader::onBeacon(WaveShortMessage* wsm)
{
    // vehicles other than CACC should ignore the received beacon
    if( !isCACC() )
        return;

    EV << "## " << SUMOvID << " received beacon ..." << std::endl;
    printBeaconContent(wsm);

    // I am platoon leader and I do not care about the received beacon!
    if(myPlatoonDepth == 0)
        return;

    bool update = false;

    // I am not part of any platoon yet!
    if(platoonID == -1)
    {
        if(wsm->getPlatoonID() != -1 && wsm->getPlatoonDepth() == 0)
        {
            EV << "This beacon is from a platoon leader. I will join ..." << std::endl;

            platoonID = wsm->getPlatoonID();
            platoonLeaderID = wsm->getSender();
            update = true;
        }
    }
    // platoonID != -1 which means I am already part of a platoon
    // if the beacon is from my own platoon
    else if(platoonID == wsm->getPlatoonID())
    {
        // if the beacon is from my platoon leader
        if(wsm->getPlatoonDepth() == 0)
        {
            EV << "This beacon is from my platoon leader ..." << std::endl;

            // update the platoonLeaderID
            platoonLeaderID = wsm->getSender();
            update = true;
        }
    }
    // I received a beacon from another platoon
    else if(platoonID != wsm->getPlatoonID())
    {
        // just ignore the beacon msg
    }

    // update the parameters of this vehicle in SUMO
    if(update)
    {
        char buffer [100];
        sprintf (buffer, "%f#%f#%f#%f", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (simTime().dbl())*1000);
        manager->commandSetPreceding(SUMOvID, buffer);
        // manager->commandSetPlatoonLeader(SUMOvID, buffer);

        // a beacon from platoon leader is received
        data *pair = new data(wsm->getSender());
        simsignal_t Signal_beaconP = registerSignal("beaconP");
        nodePtr->emit(Signal_beaconP, pair);
    }
    else
    {
        // a beacon from other vehicles is received
        data *pair = new data(wsm->getSender());
        simsignal_t Signal_beaconO = registerSignal("beaconO");
        nodePtr->emit(Signal_beaconO, pair);
    }
}


void ApplVBeaconPlatoonLeader::onData(WaveShortMessage* wsm)
{
    error("ApplVBeaconPlatoonLeader can not handle data. Something is wrong!");
}


// is called, every time the position of vehicle changes
void ApplVBeaconPlatoonLeader::handlePositionUpdate(cObject* obj)
{
    ApplVBeacon::handlePositionUpdate(obj);
}


bool ApplVBeaconPlatoonLeader::isBeaconFromPlatoonLeader(WaveShortMessage* wsm)
{
    // check if a platoon leader is sending this
    if( wsm->getPlatoonDepth() == 0 )
    {
        // check if this is actually my platoon leader
        if( wsm->getPlatoonID() == platoonID)
        {
            return true;
        }
    }

    return false;
}


void ApplVBeaconPlatoonLeader::finish()
{

}


ApplVBeaconPlatoonLeader::~ApplVBeaconPlatoonLeader()
{

}


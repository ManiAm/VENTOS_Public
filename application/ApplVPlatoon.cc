
#include "ApplVPlatoon.h"

Define_Module(ApplVPlatoon);

void ApplVPlatoon::initialize(int stage)
{
    ApplVBeacon::initialize(stage);

	if (stage == 0)
	{
	    one_vehicle_look_ahead = par("one_vehicle_look_ahead");
	    platoonLeaderID = par("platoonLeaderID").stringValue();
	    platoonID = par("platoonID").longValue();

	    if(SUMOvID == platoonLeaderID)
	    {
	        isPlatoonLeader = true;
	    }
	    // if I am not the platoon leader, then I should wait until
	    // I receive a beacon from platoon leader
	    else
	    {
	        isPlatoonLeader = false;
	        platoonID = -1;
	        platoonLeaderID = "";
	    }
	}
}


void ApplVPlatoon::handleLowerMsg(cMessage* msg)
{
    ApplVBeacon::handleLowerMsg(msg);

}


void ApplVPlatoon::handleSelfMsg(cMessage* msg)
{
    switch (msg->getKind())
    {
        case SEND_BEACON_EVT:
        {
            WaveShortMessage* Msg = ApplVBeacon::prepareBeacon("beacon", beaconLengthBits, type_CCH, beaconPriority, 0);

            // fill-in the fields related to platoon
            WaveShortMessage* beaconMsg = fillBeaconPlatoon(Msg);

            EV << "## Created beacon msg for vehicle: " << SUMOvID << std::endl;
            printBeaconContent(beaconMsg);

            // send it
            sendWSM(beaconMsg);

            // schedule for next beacon broadcast
            scheduleAt(simTime() + beaconInterval, sendBeaconEvt);

            break;
        }
    }
}


WaveShortMessage* ApplVPlatoon::fillBeaconPlatoon(WaveShortMessage *wsm)
{
    wsm->setPlatoonID(platoonID);
    wsm->setIsPlatoonLeader(isPlatoonLeader);

    return wsm;
}


void ApplVPlatoon::onBeacon(WaveShortMessage* wsm)
{
    // vehicles other than CACC should ignore the received beacon
    if(SUMOvType != "TypeCACC1" && SUMOvType != "TypeCACC2")
        return;

    // in one_vehicle_look_ahead (i.e. No platooning) we should only
    // get information from our proceeding vehicle
    if(one_vehicle_look_ahead)
    {
        ApplVBeacon::onBeacon(wsm);
        return;
    }

    EV << "## " << SUMOvID << " received beacon ..." << std::endl;
    printBeaconContent(wsm);

    bool update = false;

    if(!isPlatoonLeader)
    {
        // I am not part of any platoon yet!
        if(platoonID == -1)
        {
            if(wsm->getPlatoonID() != -1 && wsm->getIsPlatoonLeader())
            {
                EV << "This beacon is from a platoon leader. I will join ..." << std::endl;

                platoonID = wsm->getPlatoonID();
                platoonLeaderID = wsm->getSender();
                update = true;
            }
        }
        // I am already part of a platoon
        else if(platoonID == wsm->getPlatoonID())
        {
            // update the platoonLeaderID
            if(wsm->getIsPlatoonLeader())
            {
                EV << "This beacon is from my platoon leader ..." << std::endl;

                platoonLeaderID = wsm->getSender();
                update = true;
            }
        }
        // I received a beacon from another platoon
        else if(platoonID != wsm->getPlatoonID())
        {
            // just ignore the beacon msg
        }
    }

    // update the parameters of this vehicle in SUMO
    if(update)
    {
        char buffer [100];
        sprintf (buffer, "%f#%f#%f", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel());

        manager->commandSetPreceding(SUMOvID, buffer);
        //manager->commandSetPlatoonLeader(SUMOvID, buffer);
     }
}


void ApplVPlatoon::onData(WaveShortMessage* wsm)
{

}


// is called, every time the position of vehicle changes
void ApplVPlatoon::handlePositionUpdate(cObject* obj)
{
    ApplVBeacon::handlePositionUpdate(obj);


}


void ApplVPlatoon::finish()
{

}


ApplVPlatoon::~ApplVPlatoon()
{

}


#include "ApplV_02_Beacon.h"

Define_Module(ApplVBeacon);

void ApplVBeacon::initialize(int stage)
{
    ApplVBase::initialize(stage);

	if (stage == 0)
	{
	    // NED
	    if(SUMOvType != "TypeObstacle")
	    {
            VANETenabled = par("VANETenabled").boolValue();
	    }
	    else
	    {
	        VANETenabled = false;
	    }

        mode = par("mode").longValue();
        sonarDist = par("sonarDist").doubleValue();

        // NED variables (beaconing parameters)
        sendBeacons = par("sendBeacons").boolValue();
        beaconInterval = par("beaconInterval").doubleValue();
        maxOffset = par("maxOffset").doubleValue();
        beaconLengthBits = par("beaconLengthBits").longValue();
        beaconPriority = par("beaconPriority").longValue();

        // NED variables (data parameters)
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        // simulate asynchronous channel access
        double offSet = dblrand() * (beaconInterval/2);
        offSet = offSet + floor(offSet/0.050)*0.050;
        individualOffset = dblrand() * maxOffset;

        VehicleBeaconEvt = new cMessage("BeaconEvt", KIND_TIMER);
        if (VANETenabled && sendBeacons)
        {
            scheduleAt(simTime() + offSet, VehicleBeaconEvt);
        }

        pauseBeaconing = false;

        plnID = "";
        myPlnDepth = -1;
        plnSize = -1;
        plnMembersList.clear();

        // pre-defined platoon
        if(mode == 3)
        {
            preDefinedPlatoonID = par("preDefinedPlatoonID").stringValue();

            // I am the platoon leader.
            if(SUMOvID == preDefinedPlatoonID)
            {
                // we only set these two!
                plnID = SUMOvID;
                myPlnDepth = 0;
            }
        }
	}
}


void ApplVBeacon::handleSelfMsg(cMessage* msg)
{
    ApplVBase::handleSelfMsg(msg);

    if (msg == VehicleBeaconEvt)
    {
        if(!pauseBeaconing)
        {
            BeaconVehicle* beaconMsg = prepareBeacon();

            if(mode == 2 || mode == 3 || mode == 4)
            {
                // fill-in the related fields to platoon
                beaconMsg->setPlatoonID(plnID.c_str());
                beaconMsg->setPlatoonDepth(myPlnDepth);
            }

            EV << "## Created beacon msg for vehicle: " << SUMOvID << endl;
            ApplVBeacon::printBeaconContent(beaconMsg);

            // send it
            sendDelayed(beaconMsg, individualOffset, lowerLayerOut);
        }

        // schedule for next beacon broadcast
        scheduleAt(simTime() + beaconInterval, VehicleBeaconEvt);
    }
}


BeaconVehicle*  ApplVBeacon::prepareBeacon()
{
    if (!VANETenabled)
    {
        error("Only VANETenabled vehicles can send beacon!");
    }

    BeaconVehicle* wsm = new BeaconVehicle("beaconVehicle");

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
    wsm->setSender(SUMOvID.c_str());
    wsm->setRecipient("broadcast");

    // set current position
    Coord cord = TraCI->commandGetVehiclePos(SUMOvID);
    wsm->setPos(cord);

    // set current speed
    wsm->setSpeed( TraCI->commandGetVehicleSpeed(SUMOvID) );

    // set current acceleration
    wsm->setAccel( TraCI->commandGetVehicleAccel(SUMOvID) );

    // set maxDecel
    wsm->setMaxDecel( TraCI->commandGetVehicleMaxDecel(SUMOvID) );

    // set current lane
    wsm->setLane( TraCI->getCommandInterface()->getLaneId(SUMOvID).c_str() );

    return wsm;
}


// print beacon fields (for debugging purposes)
void ApplVBeacon::printBeaconContent(BeaconVehicle* wsm)
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
    EV << wsm->getPos() << " | ";
    EV << wsm->getSpeed() << " | ";
    EV << wsm->getAccel() << " | ";
    EV << wsm->getMaxDecel() << " | ";
    EV << wsm->getLane() << " | ";
    EV << wsm->getPlatoonID() << " | ";
    EV << wsm->getPlatoonDepth() << endl;
}


// is called, every time the position of vehicle changes
void ApplVBeacon::handlePositionUpdate(cObject* obj)
{
    ApplVBase::handlePositionUpdate(obj);
}


bool ApplVBeacon::isBeaconFromLeading(BeaconVehicle* wsm)
{
    // step 1: check if a leading vehicle is present

    vector<string> vleaderIDnew = TraCI->commandGetLeading(SUMOvID, sonarDist);
    string vleaderID = vleaderIDnew[0];
    double gap = atof( vleaderIDnew[1].c_str() );

    if(vleaderID == "")
    {
        EV << "This vehicle has no leading vehicle." << endl;
        return false;
    }

    // step 2: is it on the same lane?

    string myLane = TraCI->getCommandInterface()->getLaneId(SUMOvID);
    string beaconLane = wsm->getLane();

    EV << "I am on lane " << TraCI->getCommandInterface()->getLaneId(SUMOvID) << ", and other vehicle is on lane " << wsm->getLane() << endl;

    if( myLane != beaconLane )
    {
        EV << "Not on the same lane!" << endl;
        return false;
    }

    EV << "We are on the same lane!" << endl;

    // step 3: is the distance equal to gap?

    Coord cord = TraCI->commandGetVehiclePos(SUMOvID);
    double dist = sqrt( pow(cord.x - wsm->getPos().x, 2) +  pow(cord.y - wsm->getPos().y, 2) );

    // subtract the length of the leading vehicle from dist
    dist = dist - TraCI->commandGetVehicleLength(vleaderID);

    EV << "my coord (x,y): " << cord.x << "," << cord.y << endl;
    EV << "other coord (x,y): " << wsm->getPos().x << "," << wsm->getPos().y << endl;
    EV << "distance is " << dist << ", and gap is " << gap << endl;

    double diff = fabs(dist - gap);

    if(diff > 0.001)
    {
        EV << "distance does not match the gap!" << endl;
        return false;
    }

    if(cord.x > wsm->getPos().x)
    {
        EV << "beacon is coming from behind!" << endl;
        return false;
    }

    EV << "This beacon is from the leading vehicle!" << endl;
    return true;
}


bool ApplVBeacon::isBeaconFromMyPlatoonLeader(BeaconVehicle* wsm)
{
    // check if a platoon leader is sending this
    if( wsm->getPlatoonDepth() == 0 )
    {
        // check if this is actually my platoon leader
        if( string(wsm->getPlatoonID()) == plnID)
        {
            return true;
        }
    }

    return false;
}


void ApplVBeacon::finish()
{
    if (VehicleBeaconEvt->isScheduled())
    {
        cancelAndDelete(VehicleBeaconEvt);
    }
    else
    {
        delete VehicleBeaconEvt;
    }
}


ApplVBeacon::~ApplVBeacon()
{

}


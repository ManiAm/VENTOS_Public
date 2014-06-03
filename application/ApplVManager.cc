
#include "ApplVManager.h"

Define_Module(ApplVManager);

void ApplVManager::initialize(int stage)
{
    ApplVBeacon::initialize(stage);

	if (stage == 0)
	{
        // NED variable
        SUMOvehicleDebug = par("SUMOvehicleDebug").boolValue();
        modeSwitch = par("modeSwitch").boolValue();

        // set parameters in SUMO
        manager->commandSetDebug(SUMOvID, SUMOvehicleDebug);
        manager->commandSetModeSwitch(SUMOvID, modeSwitch);

        // NED variables (packet loss ratio)
        droppT = par("droppT").doubleValue();
        droppV = par("droppV").stringValue();
        plr = par("plr").doubleValue();

        // NED
        sonarDist = par("sonarDist").doubleValue();

        // NED variables (measurement errors)
        measurementError = par("measurementError");
        errorGap = par("errorGap");
        errorRelSpeed = par("errorRelSpeed");

        if(measurementError)
        {
            manager->commandSetErrorGap(SUMOvID, errorGap);
            manager->commandSetErrorRelSpeed(SUMOvID, errorRelSpeed);
        }
        else
        {
            manager->commandSetErrorGap(SUMOvID, 0.);
            manager->commandSetErrorRelSpeed(SUMOvID, 0.);
        }

	    // NED variables
	    mode = par("mode").longValue();
        one_vehicle_look_ahead = par("one_vehicle_look_ahead");

	    // pre-defined platoon
	    if(mode == 2)
	    {
	        preDefinedPlatoonID = par("preDefinedPlatoonID").stringValue();

	        // I am the platoon leader
	        if(SUMOvID == preDefinedPlatoonID)
	        {
	            platoonID = SUMOvID;
	            myPlatoonDepth = 0;
	            // platoonSize;
	            // queue;
	        }
	    }
	}
}


void ApplVManager::handleSelfMsg(cMessage* msg)
{
    if (msg->getKind() == SEND_BEACON_EVT)
    {
        if(mode == 0)
        {
            ApplVBeacon::handleSelfMsg(msg);
        }
        else if(mode == 1 || mode == 2 || mode == 3)
        {
            BeaconVehicle* Msg = ApplVBeacon::prepareBeacon();

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
    }
}


void ApplVManager::handleLowerMsg(cMessage* msg)
{
    // vehicles other than CACC should ignore the received msg
    if( !VANETenabled )
        return;

    // make sure msg is of type WaveShortMessage
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()) == "beaconVehicle")
    {
        BeaconVehicle* wsm = dynamic_cast<BeaconVehicle*>(msg);
        ASSERT(wsm);

        if( !dropBeacon(droppT, droppV, plr) )
        {
            ApplVManager::onBeaconVehicle(wsm);
        }
        // drop the beacon, and report it to statistics
        else
        {
            reportDropToStatistics(wsm);
        }
    }
    else if(std::string(wsm->getName()) == "platoonMsg")
    {
        PlatoonMsg* wsm = dynamic_cast<PlatoonMsg*>(msg);
        ASSERT(wsm);

        ApplVManager::onData(wsm);
    }
}


// simulate packet loss in application layer
bool ApplVManager::dropBeacon(double time, std::string vehicle, double plr)
{
    if(simTime().dbl() >= time)
    {
        // vehicle == "" --> drop beacon in all vehicles
        // vehicle == SUMOvID --> drop beacon only in specified vehicle
        if (vehicle == "" || vehicle == SUMOvID)
        {
            // random number in [0,1)
            double p = dblrand();

            if( p < (plr/100) )
                return true;   // drop the beacon
            else
                return false;  // keep the beacon
        }
        else
            return false;
    }
    else
        return false;
}


void ApplVManager::reportDropToStatistics(BeaconVehicle* wsm)
{
    // todo:
    if(one_vehicle_look_ahead)
    {
        bool result =  ApplVManager::isBeaconFromLeading(wsm);

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
    else
    {
        bool result = ApplVManager::isBeaconFromMyPlatoonLeader(wsm);

        // todo:
        if(result)
        {
            // a beacon from platoon leader is drooped
            simsignal_t Signal_beaconD = registerSignal("beaconD");
            nodePtr->emit(Signal_beaconD, 3);
        }
        else
        {
            // a beacon from other vehicle is drooped
            simsignal_t Signal_beaconD = registerSignal("beaconD");
            nodePtr->emit(Signal_beaconD, 2);
        }
    }
}


void ApplVManager::onBeaconVehicle(BeaconVehicle* wsm)
{
    EV << "## " << SUMOvID << " received beacon ..." << endl;
    ApplVBeacon::printBeaconContent(wsm);

    bool result = false;

    // stand-alone vehicle
    // ignore the received beacon!
    if(mode == 0)
    {

    }
    // CACC stream
    else if(mode == 1)
    {
        result = ApplVManager::isBeaconFromLeading(wsm);
    }
    // predefined platoon
    else if(mode == 2)
    {
        // I am platoon leader
        // get data from my leading vehicle
        if(myPlatoonDepth == 0)
        {
            result = ApplVManager::isBeaconFromLeading(wsm);
        }
        // I am follower
        else
        {
            // get data from my leading vehicle
            if(one_vehicle_look_ahead)
            {
                result = ApplVManager::isBeaconFromLeading(wsm);
            }
            // get data from the platoon leader
            else
            {
                result = ApplVManager::isBeaconFromMyPlatoonLeader(wsm);
            }

            // I am not part of any platoon yet!
            if(platoonID == "")
            {
                if( std::string(wsm->getPlatoonID()) != "" && wsm->getPlatoonDepth() == 0 )
                {
                    EV << "This beacon is from a platoon leader. I will join ..." << std::endl;
                    platoonID = wsm->getPlatoonID();

                    // change the color to blue
                    TraCIColor newColor = TraCIColor::fromTkColor("blue");
                    manager->commandSetVehicleColor(SUMOvID, newColor);
                }
            }
            // platoonID != "" which means I am already part of a platoon
            // if the beacon is from my platoon leader
            else if( isBeaconFromMyPlatoonLeader(wsm) )
            {
                // do nothing!
                EV << "This beacon is from my platoon leader ..." << std::endl;
            }
            // I received a beacon from another platoon
            else if( std::string(wsm->getPlatoonID()) != platoonID )
            {
                // ignore the beacon msg
            }
        }
    }
    // platoon formation
    else if(mode == 3)
    {
        // I am platoon leader
        // get data from my leading vehicle
        if(myPlatoonDepth == 0)
        {
            result = ApplVManager::isBeaconFromLeading(wsm);
        }
        // I am follower
        // get data from my leading vehicle
        else if(one_vehicle_look_ahead)
        {
            result = ApplVManager::isBeaconFromLeading(wsm);
        }
        // I am follower
        // get data from the platoon leader
        else
        {
            result = ApplVManager::isBeaconFromMyPlatoonLeader(wsm);
        }
    }

    // send results to SUMO if result = true
    if( result && isCACCvehicle() )
    {
        char buffer [200];
        sprintf (buffer, "%f#%f#%f#%f#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (simTime().dbl())*1000, wsm->getSender() );
        manager->commandSetCFParameters(SUMOvID, buffer);

        // a beacon from the leading vehicle or platoon leader is received
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


void ApplVManager::onData(PlatoonMsg* wsm)
{
    // todo:

    if(mode == 3)
    {

    }
    // ApplVPlatoonFormation2::onData(wsm);
}


bool ApplVManager::isBeaconFromLeading(BeaconVehicle* wsm)
{
    // step 1: check if a leading vehicle is present

    std::vector<std::string> vleaderIDnew = manager->commandGetLeading(SUMOvID, sonarDist);
    std::string vleaderID = vleaderIDnew[0];
    double gap = atof( vleaderIDnew[1].c_str() );

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

    EV << "my coord (x,y): " << cord.x << "," << cord.y << std::endl;
    EV << "other coord (x,y): " << wsm->getPos().x << "," << wsm->getPos().y << std::endl;
    EV << "distance is " << dist << ", and gap is " << gap << std::endl;

    double diff = fabs(dist - gap);

    if(diff > 0.001)
    {
        EV << "distance does not match the gap!" << std::endl;
        return false;
    }

    if(cord.x > wsm->getPos().x)
    {
        EV << "beacon is coming from behind!" << std::endl;
        return false;
    }

    EV << "This beacon is from the leading vehicle!" << std::endl;
    return true;
}


bool ApplVManager::isBeaconFromMyPlatoonLeader(BeaconVehicle* wsm)
{
    // check if a platoon leader is sending this
    if( wsm->getPlatoonDepth() == 0 )
    {
        // check if this is actually my platoon leader
        if( std::string(wsm->getPlatoonID()) == platoonID)
        {
            return true;
        }
    }

    return false;
}


// is called, every time the position of vehicle changes
void ApplVManager::handlePositionUpdate(cObject* obj)
{
    ApplVBeacon::handlePositionUpdate(obj);
}


void ApplVManager::finish()
{
    ApplVBeacon::finish();
}


ApplVManager::~ApplVManager()
{

}



#include "ApplV_06_Manager.h"

Define_Module(ApplVManager);

void ApplVManager::initialize(int stage)
{
    ApplVPlatoonMg::initialize(stage);

	if (stage == 0)
	{
        // NED variables
        SUMOvehicleDebug = par("SUMOvehicleDebug").boolValue();
        modeSwitch = par("modeSwitch").boolValue();
        sonarDist = par("sonarDist").doubleValue();

        // set parameters in SUMO
        TraCI->commandSetDebug(SUMOvID, SUMOvehicleDebug);
        TraCI->commandSetModeSwitch(SUMOvID, modeSwitch);

        // NED variables (packet loss ratio)
        droppT = par("droppT").doubleValue();
        droppV = par("droppV").stringValue();
        plr = par("plr").doubleValue();

        // NED variables (measurement errors)
        measurementError = par("measurementError").boolValue();
        errorGap = par("errorGap").doubleValue();
        errorRelSpeed = par("errorRelSpeed").doubleValue();

        if(measurementError)
        {
            TraCI->commandSetErrorGap(SUMOvID, errorGap);
            TraCI->commandSetErrorRelSpeed(SUMOvID, errorRelSpeed);
        }
        else
        {
            TraCI->commandSetErrorGap(SUMOvID, 0.);
            TraCI->commandSetErrorRelSpeed(SUMOvID, 0.);
        }

	    // NED variables
        one_vehicle_look_ahead = par("one_vehicle_look_ahead");
	}
}


void ApplVManager::handleSelfMsg(cMessage* msg)
{
    ApplVPlatoonMg::handleSelfMsg(msg);
}


void ApplVManager::handleLowerMsg(cMessage* msg)
{
    // vehicles other than CACC should ignore the received msg
    if( !VANETenabled )
        return;

    // make sure msg is of type WaveShortMessage
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (string(wsm->getName()) == "beaconVehicle")
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
    else if (string(wsm->getName()) == "beaconRSU")
    {
        BeaconRSU* wsm = dynamic_cast<BeaconRSU*>(msg);
        ASSERT(wsm);

        ApplVManager::onBeaconRSU(wsm);
    }
    else if(string(wsm->getName()) == "platoonMsg")
    {
        PlatoonMsg* wsm = dynamic_cast<PlatoonMsg*>(msg);
        ASSERT(wsm);

        ApplVManager::onData(wsm);
    }

    delete msg;
}


// simulate packet loss in application layer
bool ApplVManager::dropBeacon(double time, string vehicle, double plr)
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
        bool result =  isBeaconFromLeading(wsm);

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
        bool result = isBeaconFromMyPlatoonLeader(wsm);

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
    // pass it down
    ApplVPlatoonMg::onBeaconVehicle(wsm);

    EV << "## " << SUMOvID << " received beacon ..." << endl;
    ApplVBeacon::printBeaconContent(wsm);

    bool result = false;

    // stand-alone vehicle
    // ignore the received beacon!
    if(mode == 1)
    {

    }
    // CACC stream
    else if(mode == 2)
    {
        result = isBeaconFromLeading(wsm);
    }
    // predefined platoon
    else if(mode == 3)
    {
        // I am platoon leader
        // get data from my leading vehicle
        if(myPlnDepth == 0)
        {
            result = isBeaconFromLeading(wsm);
        }
        // I am follower
        else
        {
            // get data from my leading vehicle
            if(one_vehicle_look_ahead)
            {
                result = isBeaconFromLeading(wsm);
            }
            // get data from the platoon leader
            else
            {
                result = isBeaconFromMyPlatoonLeader(wsm);
            }

            // I am not part of any platoon yet!
            if(plnID == "")
            {
                if( string(wsm->getPlatoonID()) != "" && wsm->getPlatoonDepth() == 0 )
                {
                    EV << "This beacon is from a platoon leader. I will join ..." << endl;
                    plnID = wsm->getPlatoonID();

                    // change the color to blue
                    TraCIColor newColor = TraCIColor::fromTkColor("blue");
                    TraCI->getCommandInterface()->setColor(SUMOvID, newColor);
                }
            }
            // platoonID != "" which means I am already part of a platoon
            // if the beacon is from my platoon leader
            else if( isBeaconFromMyPlatoonLeader(wsm) )
            {
                // do nothing!
                EV << "This beacon is from my platoon leader ..." << endl;
            }
            // I received a beacon from another platoon
            else if( string(wsm->getPlatoonID()) != plnID )
            {
                // ignore the beacon msg
            }
        }
    }
    // platoon formation
    else if(mode == 4)
    {
        // I am platoon leader
        // get data from my leading vehicle
        if(myPlnDepth == 0)
        {
            result = isBeaconFromLeading(wsm);
        }
        // I am follower
        // get data from my leading vehicle
        else if(one_vehicle_look_ahead)
        {
            result = isBeaconFromLeading(wsm);
        }
        // I am follower
        // get data from the platoon leader
        else
        {
            result = isBeaconFromMyPlatoonLeader(wsm);
        }
    }
    else
    {
        error("not a valid mode!");
    }

    // send results to SUMO if result = true
    if(result)
    {
        char buffer [200];
        sprintf (buffer, "%f#%f#%f#%f#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (simTime().dbl())*1000, wsm->getSender() );
        TraCI->commandSetCFParameters(SUMOvID, buffer);

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


void ApplVManager::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down
    ApplVPlatoonMg::onBeaconRSU(wsm);


}


void ApplVManager::onData(PlatoonMsg* wsm)
{
    // pass it down
    ApplVPlatoonMg::onData(wsm);

}


bool ApplVManager::isBeaconFromLeading(BeaconVehicle* wsm)
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


bool ApplVManager::isBeaconFromMyPlatoonLeader(BeaconVehicle* wsm)
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


// is called, every time the position of vehicle changes
void ApplVManager::handlePositionUpdate(cObject* obj)
{
    ApplVPlatoonMg::handlePositionUpdate(obj);
}


void ApplVManager::finish()
{
    ApplVPlatoonMg::finish();
}


ApplVManager::~ApplVManager()
{

}


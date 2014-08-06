
#include "ApplV_07_Manager.h"

namespace VENTOS {

Define_Module(VENTOS::ApplVManager);

void ApplVManager::initialize(int stage)
{
    ApplVPlatoonMg::initialize(stage);

	if (stage == 0)
	{
        // NED variables
        SUMOvehicleDebug = par("SUMOvehicleDebug").boolValue();
        modeSwitch = par("modeSwitch").boolValue();

        // set parameters in SUMO
        TraCI->commandSetDebug(SUMOvID, SUMOvehicleDebug);
        TraCI->commandSetModeSwitch(SUMOvID, modeSwitch);
        TraCI->commandSetControlMode(SUMOvID, controlMode);

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
    if(true /*one_vehicle_look_ahead*/)
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
    if(controlMode == 1)
    {

    }
    // one-vehicle look-ahead
    else if(controlMode == 2)
    {
        if( isBeaconFromLeading(wsm) )
        {
            char buffer [200];
            sprintf (buffer, "%f#%f#%f#%f#%s#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (simTime().dbl())*1000, wsm->getSender(), "preceding");
            TraCI->commandSetCFParameters(SUMOvID, buffer);
        }
    }
    // from platoon leader
    else if(controlMode == 3)
    {
        if(plnMode == 1)
            error("no platoon leader is present! check plnMode!");

        // I am platoon leader
        // get data from my leading vehicle
        if(myPlnDepth == 0)
        {
            if( isBeaconFromLeading(wsm) )
            {
                char buffer [200];
                sprintf (buffer, "%f#%f#%f#%f#%s#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (simTime().dbl())*1000, wsm->getSender(), "preceding");
                TraCI->commandSetCFParameters(SUMOvID, buffer);
            }
        }
        // I am a follower
        // get data from my platoon leader
        else
        {
            if( isBeaconFromMyPlatoonLeader(wsm) )
            {
                char buffer [200];
                sprintf (buffer, "%f#%f#%f#%f#%s#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (simTime().dbl())*1000, wsm->getSender(), "leader");
                TraCI->commandSetCFParameters(SUMOvID, buffer);
            }
        }
    }
    else
    {
        error("not a valid control mode!");
    }





//    // send results to SUMO if result = true
//    if(result)
//    {
//        char buffer [200];
//        sprintf (buffer, "%f#%f#%f#%f#%s", (double)wsm->getSpeed(), (double)wsm->getAccel(), (double)wsm->getMaxDecel(), (simTime().dbl())*1000, wsm->getSender() );
//        TraCI->commandSetCFParameters(SUMOvID, buffer);
//
//        // a beacon from the leading vehicle or platoon leader is received
//        data *pair = new data(wsm->getSender());
//        simsignal_t Signal_beaconP = registerSignal("beaconP");
//        nodePtr->emit(Signal_beaconP, pair);
//    }
//    else
//    {
//        // a beacon from other vehicles is received
//        data *pair = new data(wsm->getSender());
//        simsignal_t Signal_beaconO = registerSignal("beaconO");
//        nodePtr->emit(Signal_beaconO, pair);
//    }
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

}


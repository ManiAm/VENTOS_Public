
#include "ApplBike_03_Manager.h"

namespace VENTOS {

Define_Module(VENTOS::ApplBikeManager);

ApplBikeManager::~ApplBikeManager()
{

}


void ApplBikeManager::initialize(int stage)
{
    ApplBikeBeacon::initialize(stage);

	if (stage == 0)
	{
        // NED variables
        SUMOvehicleDebug = par("SUMOvehicleDebug").boolValue();

        // set parameters in SUMO
        TraCI->commandSetVehicleDebug(SUMOvID, SUMOvehicleDebug);
	}
}


void ApplBikeManager::finish()
{
    ApplBikeBeacon::finish();
}


void ApplBikeManager::handleSelfMsg(cMessage* msg)
{
    ApplBikeBeacon::handleSelfMsg(msg);
}


void ApplBikeManager::handleLowerMsg(cMessage* msg)
{
    // make sure msg is of type WaveShortMessage
    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (string(wsm->getName()) == "beaconVehicle")
    {
        BeaconVehicle* wsm = dynamic_cast<BeaconVehicle*>(msg);
        ASSERT(wsm);

    }
    else if (string(wsm->getName()) == "beaconRSU")
    {
        BeaconRSU* wsm = dynamic_cast<BeaconRSU*>(msg);
        ASSERT(wsm);

    }
    else if(string(wsm->getName()) == "platoonMsg")
    {
        PlatoonMsg* wsm = dynamic_cast<PlatoonMsg*>(msg);
        ASSERT(wsm);

        ApplBikeManager::onData(wsm);
    }

    delete msg;
}


void ApplBikeManager::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down
    //ApplBikeBeacon::onBeaconVehicle(wsm);

}


void ApplBikeManager::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down
    //ApplBikeBeacon::onBeaconRSU(wsm);
}


void ApplBikeManager::onData(PlatoonMsg* wsm)
{
    // pass it down
    //ApplBikeBeacon::onData(wsm);
}


// is called, every time the position of vehicle changes
void ApplBikeManager::handlePositionUpdate(cObject* obj)
{
    // pass it down
    ApplBikeBeacon::handlePositionUpdate(obj);
}


}


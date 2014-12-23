
#include "ApplPed_03_Manager.h"

namespace VENTOS {

Define_Module(VENTOS::ApplPedManager);

ApplPedManager::~ApplPedManager()
{

}


void ApplPedManager::initialize(int stage)
{
    ApplPedBeacon::initialize(stage);

	if (stage == 0)
	{
        // NED variables
        SUMOvehicleDebug = par("SUMOvehicleDebug").boolValue();

        // set parameters in SUMO
        TraCI->commandSetDebug(SUMOvID, SUMOvehicleDebug);
	}
}


void ApplPedManager::finish()
{
    ApplPedBeacon::finish();
}


void ApplPedManager::handleSelfMsg(cMessage* msg)
{
    ApplPedBeacon::handleSelfMsg(msg);
}


void ApplPedManager::handleLowerMsg(cMessage* msg)
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

        ApplPedManager::onData(wsm);
    }

    delete msg;
}


void ApplPedManager::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down
    //ApplPedBeacon::onBeaconVehicle(wsm);

}


void ApplPedManager::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down
    //ApplPedBeacon::onBeaconRSU(wsm);
}


void ApplPedManager::onData(PlatoonMsg* wsm)
{
    // pass it down
    //ApplPedBeacon::onData(wsm);
}


// is called, every time the position of vehicle changes
void ApplPedManager::handlePositionUpdate(cObject* obj)
{
    // pass it down
    ApplPedBeacon::handlePositionUpdate(obj);
}


}


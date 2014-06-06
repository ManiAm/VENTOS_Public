
#include "ApplV_05_PlatoonMg.h"

Define_Module(ApplVPlatoonMg);

void ApplVPlatoonMg::initialize(int stage)
{
    ApplV_AID::initialize(stage);

	if (stage == 0)
	{

	}
}


// handle my own SelfMsg
void ApplVPlatoonMg::handleSelfMsg(cMessage* msg)
{
    ApplV_AID::handleSelfMsg(msg);
}


void ApplVPlatoonMg::onBeaconVehicle(BeaconVehicle* wsm)
{
    ApplV_AID::onBeaconVehicle(wsm);

}


void ApplVPlatoonMg::onBeaconRSU(BeaconRSU* wsm)
{
    ApplV_AID::onBeaconRSU(wsm);

}


void ApplVPlatoonMg::onData(PlatoonMsg* wsm)
{
    ApplV_AID::onData(wsm);

}

// is called, every time the position of vehicle changes
void ApplVPlatoonMg::handlePositionUpdate(cObject* obj)
{
    ApplV_AID::handlePositionUpdate(obj);
}


void ApplVPlatoonMg::finish()
{
    ApplV_AID::finish();
}


ApplVPlatoonMg::~ApplVPlatoonMg()
{

}


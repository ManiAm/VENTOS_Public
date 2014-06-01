
#include "ApplVPlatoonFormation3.h"

Define_Module(ApplVPlatoonFormation3);

void ApplVPlatoonFormation3::initialize(int stage)
{
    ApplVPlatoonFormation2::initialize(stage);

	if (stage == 0)
	{

	}
}


void ApplVPlatoonFormation3::handleLowerMsg(cMessage* msg)
{
    error("ApplVPlatoonFormation3 should not receive any lower message!");
}


void ApplVPlatoonFormation3::handleSelfMsg(cMessage* msg)
{
    ApplVPlatoonFormation2::handleSelfMsg(msg);


}


void ApplVPlatoonFormation3::onBeaconVehicle(BeaconVehicle* wsm)
{
    ApplVPlatoonFormation2::onBeaconVehicle(wsm);


}


void ApplVPlatoonFormation3::onData(PlatoonMsg* wsm)
{
    ApplVPlatoonFormation2::onData(wsm);


}


// is called, every time the position of vehicle changes
void ApplVPlatoonFormation3::handlePositionUpdate(cObject* obj)
{
    ApplVPlatoonFormation2::handlePositionUpdate(obj);
}


void ApplVPlatoonFormation3::FSMchangeState()
{
    ApplVPlatoonFormation2::FSMchangeState();

}


void ApplVPlatoonFormation3::finish()
{
    ApplVPlatoonFormation2::finish();

}


ApplVPlatoonFormation3::~ApplVPlatoonFormation3()
{

}

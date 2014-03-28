
#include "ApplVPlatoonFormation2.h"

Define_Module(ApplVPlatoonFormation2);

void ApplVPlatoonFormation2::initialize(int stage)
{
    ApplVPlatoonFormation::initialize(stage);

	if (stage == 0)
	{

	}
}


void ApplVPlatoonFormation2::handleLowerMsg(cMessage* msg)
{
    error("ApplVPlatoonFormation2 should not receive any lower message!");
}


void ApplVPlatoonFormation2::handleSelfMsg(cMessage* msg)
{
    ApplVPlatoonFormation::handleSelfMsg(msg);


}


void ApplVPlatoonFormation2::onBeacon(Beacon* wsm)
{
    ApplVPlatoonFormation::onBeacon(wsm);


}


void ApplVPlatoonFormation2::onData(PlatoonMsg* wsm)
{
    ApplVPlatoonFormation::onData(wsm);



}


// is called, every time the position of vehicle changes
void ApplVPlatoonFormation2::handlePositionUpdate(cObject* obj)
{
    ApplVPlatoonFormation::handlePositionUpdate(obj);
}


void ApplVPlatoonFormation2::FSMchangeState()
{
    ApplVPlatoonFormation::FSMchangeState();

}


void ApplVPlatoonFormation2::finish()
{

}


ApplVPlatoonFormation2::~ApplVPlatoonFormation2()
{

}


#include "ApplV_05_PlatoonMg.h"

Define_Module(ApplVPlatoonMg);

void ApplVPlatoonMg::initialize(int stage)
{
    ApplV_AID::initialize(stage);

    if(mode != 4)
        return;

	if (stage == 0)
	{
        maxPlatoonSize = par("maxPlatoonSize").longValue();


       // double timer1Value = par("timer1Value").doubleValue();
       // double timer2Value = par("timer2Value").doubleValue();

       // bool platoonLeaderLeave = par("platoonLeaderLeave").boolValue();
       // bool platoonMemberLeave = par("platoonMemberLeave").boolValue();
       // double timer3Value = par("timer3Value").doubleValue();

        vehicleState = state_idle;

        // entry maneuver
        EntryManeuverEvt = new cMessage("EntryEvt", KIND_TIMER);
        double offset = dblrand() * 10;
        scheduleAt(simTime() + offset, EntryManeuverEvt);
	}
}


// handle my own SelfMsg
void ApplVPlatoonMg::handleSelfMsg(cMessage* msg)
{
    ApplV_AID::handleSelfMsg(msg);

    if(msg == EntryManeuverEvt)
    {
        entryManeuver();
    }
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


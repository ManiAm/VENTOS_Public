
#include "04ApplV_AID.h"
#include "Messages_m.h"

Define_Module(ApplV_AID);

void ApplV_AID::initialize(int stage)
{
    ApplVBeacon::initialize(stage);

	if (stage == 0)
	{
        AID = par("AID").boolValue();

        // NED variables (data messages)
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();

        lastLane = "";
        laneChanges.clear();
	}
}


// handle my own SelfMsg
void ApplV_AID::handleSelfMsg(cMessage* msg)
{
    ApplVBeacon::handleSelfMsg(msg);
}


void ApplV_AID::onBeaconVehicle(BeaconVehicle* wsm)
{

}


void ApplV_AID::onBeaconRSU(BeaconRSU* wsm)
{

}


void ApplV_AID::onData(PlatoonMsg* wsm)
{

}

// is called, every time the position of vehicle changes
void ApplV_AID::handlePositionUpdate(cObject* obj)
{
    ApplVBeacon::handlePositionUpdate(obj);

    std::string lane = manager->commandGetLaneId(SUMOvID);

    if(lane != lastLane)
    {
        std::ostringstream str;
        Coord pos =  manager->commandGetVehiclePos(SUMOvID);
        str << simTime().dbl() << "#" << lane << "#" << pos.x << "#" << pos.y;
        laneChanges.push_back(str.str());
    }
}


void ApplV_AID::finish()
{
    ApplVBeacon::finish();
}


ApplV_AID::~ApplV_AID()
{

}


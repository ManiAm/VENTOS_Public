
#include "ApplV_05_PlatoonFormed.h"

namespace VENTOS {

Define_Module(VENTOS::ApplVPlatoonFormed);

ApplVPlatoonFormed::~ApplVPlatoonFormed()
{

}


void ApplVPlatoonFormed::initialize(int stage)
{
    ApplV_AID::initialize(stage);

	if (stage == 0)
	{
        plnMode = par("plnMode").longValue();

	    if(plnMode != 2)
	        return;

        if(!VANETenabled)
            error("This vehicle is not VANET-enabled!");

	    preDefinedPlatoonID = par("preDefinedPlatoonID").stringValue();

	    // I am the platoon leader.
	    if(SUMOvID == preDefinedPlatoonID)
	    {
	        // we only set these two!
	        plnID = SUMOvID;
	        myPlnDepth = 0;
	    }
	}
}


void ApplVPlatoonFormed::finish()
{
    ApplV_AID::finish();
}


void ApplVPlatoonFormed::handleSelfMsg(cMessage* msg)
{
    // pass it down!
    ApplV_AID::handleSelfMsg(msg);
}


void ApplVPlatoonFormed::onBeaconVehicle(BeaconVehicle* wsm)
{
    // pass it down!
    ApplV_AID::onBeaconVehicle(wsm);

    if(plnMode != 2)
        return;

    // I am not part of any platoon yet!
    if(plnID == "")
    {
        if( string(wsm->getPlatoonID()) != "" && wsm->getPlatoonDepth() == 0 )
        {
            EV << "This beacon is from a platoon leader. I will join ..." << endl;
            plnID = wsm->getPlatoonID();

            // change the color to blue
            TraCIColor newColor = TraCIColor::fromTkColor("blue");
            TraCI->commandChangeVehicleColor(SUMOvID, newColor);
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


void ApplVPlatoonFormed::onBeaconRSU(BeaconRSU* wsm)
{
    // pass it down!
    ApplV_AID::onBeaconRSU(wsm);
}


void ApplVPlatoonFormed::onData(PlatoonMsg* wsm)
{
    // pass it down!
    ApplV_AID::onData(wsm);
}


// is called, every time the position of vehicle changes
void ApplVPlatoonFormed::handlePositionUpdate(cObject* obj)
{
    // pass it down!
    ApplV_AID::handlePositionUpdate(obj);
}

}


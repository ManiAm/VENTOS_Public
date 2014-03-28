
#include "ApplVBase.h"

const simsignalwrap_t ApplVBase::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

void ApplVBase::initialize(int stage)
{
	BaseApplLayer::initialize(stage);

	if (stage==0)
	{
        // get the ptr of the current module
        nodePtr = FindModule<>::findHost(this);
        if(nodePtr == NULL)
            error("can not get a pointer to the module.");

		myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(getParentModule());
		assert(myMac);

        traci = TraCIMobilityAccess().get(getParentModule());
        manager = FindModule<TraCI_Extend*>::findGlobalModule();

        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        sonarDist = par("sonarDist").doubleValue();

        headerLength = par("headerLength").longValue();

        findHost()->subscribe(mobilityStateChangedSignal, this);

        // vehicle id in omnet++
		myId = getParentModule()->getIndex();

		myFullId = getParentModule()->getFullName();

        // vehicle id in sumo
        SUMOvID = traci->getExternalId();

        // vehicle type in sumo
        SUMOvType = manager->commandGetVehicleType(SUMOvID);
	}
}


void ApplVBase::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj)
{
    Enter_Method_Silent();

    if (signalID == mobilityStateChangedSignal)
    {
        handlePositionUpdate(obj);
    }
}


void ApplVBase::handleLowerMsg(cMessage* msg)
{

}


void ApplVBase::handleSelfMsg(cMessage* msg)
{


}


void ApplVBase::handlePositionUpdate(cObject* obj)
{
    ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
}


void ApplVBase::sendWSM(Beacon* wsm)
{
    error("sendWSM of ApplVBase should not be called!");
}


bool ApplVBase::isCACC()
{
    // todo: correct this!
    return true;

    if(SUMOvType == "TypeCACC" || SUMOvType == "TypeCACC0" || SUMOvType == "TypeCACC1" || SUMOvType == "TypeCACC2")
        return true;
    else
        return false;
}


void ApplVBase::finish()
{
	findHost()->unsubscribe(mobilityStateChangedSignal, this);
}


ApplVBase::~ApplVBase()
{

}


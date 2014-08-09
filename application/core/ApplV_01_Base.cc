
#include "ApplV_01_Base.h"

namespace VENTOS {

const simsignalwrap_t ApplVBase::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

Define_Module(VENTOS::ApplVBase);

ApplVBase::~ApplVBase()
{

}


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

		TraCI_Mobility = TraCIMobilityAccess().get(getParentModule());
        TraCI = FindModule<TraCI_Extend*>::findGlobalModule();

        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        headerLength = par("headerLength").longValue();

        // vehicle id in omnet++
		myId = getParentModule()->getIndex();

		myFullId = getParentModule()->getFullName();

        // vehicle id in sumo
        SUMOvID = TraCI_Mobility->getExternalId();

        // vehicle type in sumo
        SUMOvType = TraCI->commandGetVehicleType(SUMOvID);

        if(SUMOvType != "TypeObstacle")
            findHost()->subscribe(mobilityStateChangedSignal, this);
	}
}


void ApplVBase::finish()
{
    findHost()->unsubscribe(mobilityStateChangedSignal, this);
}


void ApplVBase::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj)
{
    Enter_Method_Silent();

    if (signalID == mobilityStateChangedSignal)
    {
        handlePositionUpdate(obj);
    }
}


void ApplVBase::handleSelfMsg(cMessage* msg)
{

}


void ApplVBase::handlePositionUpdate(cObject* obj)
{
    ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
}

}


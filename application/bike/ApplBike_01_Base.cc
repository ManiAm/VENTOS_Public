
#include "ApplBike_01_Base.h"

namespace VENTOS {

const simsignalwrap_t ApplBikeBase::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

Define_Module(VENTOS::ApplBikeBase);

ApplBikeBase::~ApplBikeBase()
{

}

void ApplBikeBase::initialize(int stage)
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

        // bike id in omnet++
		myId = getParentModule()->getIndex();

		myFullId = getParentModule()->getFullName();

        // bike id in sumo
        SUMObID = TraCI_Mobility->getExternalId();

        // bike type in sumo
        SUMObType = TraCI->commandGetVehicleTypeId(SUMObID);

        // store the time of entry
        entryTime = simTime().dbl();

//        if(SUMOvType != "TypeObstacle")
//            findHost()->subscribe(mobilityStateChangedSignal, this);
	}
}


void ApplBikeBase::finish()
{
    //findHost()->unsubscribe(mobilityStateChangedSignal, this);
}


void ApplBikeBase::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj)
{
    Enter_Method_Silent();

    if (signalID == mobilityStateChangedSignal)
    {
        handlePositionUpdate(obj);
    }
}


void ApplBikeBase::handleSelfMsg(cMessage* msg)
{

}


void ApplBikeBase::handlePositionUpdate(cObject* obj)
{
    ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
}

}


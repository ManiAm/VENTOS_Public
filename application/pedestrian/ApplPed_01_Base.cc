
#include "ApplPed_01_Base.h"

namespace VENTOS {

const simsignalwrap_t ApplPedBase::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

Define_Module(VENTOS::ApplPedBase);

ApplPedBase::~ApplPedBase()
{

}

void ApplPedBase::initialize(int stage)
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

        // pedestrian id in omnet++
		myId = getParentModule()->getIndex();

		myFullId = getParentModule()->getFullName();

        // pedestrian id in sumo
        SUMOpID = TraCI_Mobility->getExternalId();

        // pedestrian type in sumo
        SUMOpType = TraCI->commandGetPedestrianTypeId(SUMOpID);

        // store the time of entry
        entryTime = simTime().dbl();

//        if(SUMOvType != "TypeObstacle")
//            findHost()->subscribe(mobilityStateChangedSignal, this);
	}
}


void ApplPedBase::finish()
{
    // findHost()->unsubscribe(mobilityStateChangedSignal, this);
}


void ApplPedBase::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj)
{
    Enter_Method_Silent();

    if (signalID == mobilityStateChangedSignal)
    {
        handlePositionUpdate(obj);
    }
}


void ApplPedBase::handleSelfMsg(cMessage* msg)
{

}


void ApplPedBase::handlePositionUpdate(cObject* obj)
{
    ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
}

}


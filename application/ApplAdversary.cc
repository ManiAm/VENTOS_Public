
#include "ApplAdversary.h"

const simsignalwrap_t ApplAdversary::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

Define_Module(ApplAdversary);

void ApplAdversary::initialize(int stage)
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

        manager = FindModule<TraCI_Extend*>::findGlobalModule();

        on = par("on").boolValue();

        findHost()->subscribe(mobilityStateChangedSignal, this);

        // vehicle id in omnet++
		myId = getParentModule()->getIndex();

		myFullId = getParentModule()->getFullName();
	}
}


void ApplAdversary::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj)
{
    Enter_Method_Silent();

    if (signalID == mobilityStateChangedSignal)
    {
        handlePositionUpdate(obj);
    }
}


void ApplAdversary::handleLowerMsg(cMessage* msg)
{

}


void ApplAdversary::handleSelfMsg(cMessage* msg)
{


}


void ApplAdversary::handlePositionUpdate(cObject* obj)
{
    ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
}


void ApplAdversary::sendWSM(Beacon* wsm)
{
    error("sendWSM of ApplVBase should not be called!");
}


void ApplAdversary::finish()
{
	findHost()->unsubscribe(mobilityStateChangedSignal, this);
}


ApplAdversary::~ApplAdversary()
{

}


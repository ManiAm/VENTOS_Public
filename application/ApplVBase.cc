
#include "ApplVBase.h"

const simsignalwrap_t ApplVBase::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

void ApplVBase::initialize(int stage)
{
	BaseApplLayer::initialize(stage);

	if (stage==0)
	{
		myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(getParentModule());
		assert(myMac);

		// vehicle id in omnet++
		myId = getParentModule()->getIndex();

        findHost()->subscribe(mobilityStateChangedSignal, this);

        traci = TraCIMobilityAccess().get(getParentModule());
        manager = FindModule<MyTraCI*>::findGlobalModule();

        // vehicle id in sumo
        SUMOvID = traci->getExternalId();

        // vehicle type in sumo
        SUMOvType = manager->commandGetVehicleType(SUMOvID);

        headerLength = par("headerLength").longValue();

        // data parameters
        sendData = par("sendData").boolValue();
        dataLengthBits = par("dataLengthBits").longValue();
        dataOnSch = par("dataOnSch").boolValue();
        dataPriority = par("dataPriority").longValue();
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
    /*
    switch (msg->getKind())
    {

        default:
        {
            if (msg)
                DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
            break;
        }
    }
    */
}


void ApplVBase::handlePositionUpdate(cObject* obj)
{
    ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getCurrentPosition();
}


WaveShortMessage*  ApplVBase::prepareData(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial)
{
    return NULL;
}


void ApplVBase::sendWSM(WaveShortMessage* wsm)
{

}


void ApplVBase::finish()
{
	findHost()->unsubscribe(mobilityStateChangedSignal, this);
}


ApplVBase::~ApplVBase()
{

}



#include "ApplRSU.h"

void ApplRSU::initialize(int stage)
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

        headerLength = par("headerLength").longValue();

        // vehicle id in omnet++
		myId = getParentModule()->getIndex();

		myFullId = getParentModule()->getFullName();
	}
}


void ApplRSU::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj)
{
    Enter_Method_Silent();

}


void ApplRSU::handleLowerMsg(cMessage* msg)
{

}


void ApplRSU::handleSelfMsg(cMessage* msg)
{


}


void ApplRSU::finish()
{

}


ApplRSU::~ApplRSU()
{

}


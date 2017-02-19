
#include "Decider.h"

bool DeciderResult::isSignalCorrect() const
{
	return isCorrect;
}


Decider::Decider(DeciderToPhyInterface* phy): phy(phy), notAgain(-1) { }


omnetpp::simtime_t Decider::processSignal(AirFrame* s)
{
	return -1;
}


ChannelState Decider::getChannelState()
{
	return ChannelState();
}


omnetpp::simtime_t Decider::handleChannelSenseRequest(ChannelSenseRequest* request)
{
	return -1;
}

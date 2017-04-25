
#include "msg/AirFrame_serial.h"
#include "PERModel.h"

void PERModel::filterSignal(AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos)
{
	Signal& signal = frame->getSignal();
	//simtime_t start  = frame->getSendingTime() + signal.getPropagationDelay();
	//simtime_t end    = frame->getSendingTime() + signal.getPropagationDelay() + frame->getDuration();

	double attenuationFactor = 1;  // no attenuation
	if(packetErrorRate > 0 && omnetpp::cSimulation::getActiveSimulation()->getContext()->uniform(0, 1) < packetErrorRate)
		attenuationFactor = 0;  // absorb all energy so that the receiver cannot receive anything

	TimeMapping<Linear>* attMapping = new TimeMapping<Linear> ();
	Argument arg;
	attMapping->setValue(arg, attenuationFactor);
	signal.addAttenuation(attMapping);
}


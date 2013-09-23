
#include "ApplV.h"

Define_Module(ApplV);

void ApplV::initialize(int stage)
{
	BaseWaveApplLayer::initialize(stage);

	if (stage == 0)
	{
		traci = TraCIMobilityAccess().get(getParentModule());
		annotations = AnnotationManagerAccess().getIfExists();
		ASSERT(annotations);

		sentMessage = false;
		lastDroveAt = simTime();
	}
}

void ApplV::onBeacon(WaveShortMessage* wsm)
{

}

void ApplV::onData(WaveShortMessage* wsm)
{
	findHost()->getDisplayString().updateWith("r=16,green");
	annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), traci->getPositionAt(simTime()), "blue"));

	if (traci->getRoadId()[0] != ':') traci->commandChangeRoute(wsm->getWsmData(), 9999);
	if (!sentMessage) sendMessage(wsm->getWsmData());
}

void ApplV::sendMessage(std::string blockedRoadId)
{
	sentMessage = true;

	t_channel channel = dataOnSch ? type_SCH : type_CCH;
	WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
	wsm->setWsmData(blockedRoadId.c_str());
	sendWSM(wsm);
}


void ApplV::handlePositionUpdate(cObject* obj)
{
	BaseWaveApplLayer::handlePositionUpdate(obj);

	// stopped for for at least 10s?
	if (traci->getSpeed() < 1)
	{
		if (simTime() - lastDroveAt >= 10)
		{
			findHost()->getDisplayString().updateWith("r=16,red");
			if (!sentMessage) sendMessage(traci->getRoadId());
		}
	}
	else
	{
		lastDroveAt = simTime();
	}
}


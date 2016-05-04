
#include "SimpleObstacleShadowing.h"

#define debugEV2 EV << "PhyLayer(SimpleObstacleShadowing): "

#if 0
SimplePathlossConstMapping::SimplePathlossConstMapping(const DimensionSet& dimensions,
													   SimpleObstacleShadowing* model,
													   const double distFactor) :
	SimpleConstMapping(dimensions),
	distFactor(distFactor),
	model(model),
	hasFrequency(dimensions.hasDimension(Dimension::frequency_static()))
{
}

double SimplePathlossConstMapping::getValue(const Argument& pos) const
{
	double freq = model->carrierFrequency;
	if(hasFrequency) {
		assert(pos.hasArgVal(Dimension::frequency()));
		freq = pos.getArgValue(Dimension::frequency());
	}
	double wavelength = BaseWorldUtility::speedOfLight() / freq;
	return (wavelength * wavelength) * distFactor;
}
#endif


SimpleObstacleShadowing::SimpleObstacleShadowing(Veins::ObstacleControl& obstacleControl, double carrierFrequency, bool useTorus, const Coord& playgroundSize, bool debug) :
	obstacleControl(obstacleControl),
	carrierFrequency(carrierFrequency),
	useTorus(useTorus),
	playgroundSize(playgroundSize),
	debug(debug)
{
	if (useTorus)
	    throw omnetpp::cRuntimeError("SimpleObstacleShadowing does not work on torus-shaped playgrounds");
}


void SimpleObstacleShadowing::filterSignal(AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos)
{
    EV_STATICCONTEXT

	Signal& s = frame->getSignal();

	double factor = obstacleControl.calculateAttenuation(sendersPos, receiverPos);

	debugEV2 << "value is: " << factor << std::endl;

	bool hasFrequency = s.getTransmissionPower()->getDimensionSet().hasDimension(Dimension::frequency());
	const DimensionSet& domain = hasFrequency ? DimensionSet::timeFreqDomain() : DimensionSet::timeDomain();
	ConstantSimpleConstMapping* attMapping = new ConstantSimpleConstMapping(domain, factor);
	s.addAttenuation(attMapping);
}

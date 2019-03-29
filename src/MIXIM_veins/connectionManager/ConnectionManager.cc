
#include <cmath>
#include "ConnectionManager.h"
#include "global/BaseWorldUtility.h"

Define_Module(ConnectionManager);

double ConnectionManager::calcInterfDist()
{
    double maxIntfDist = par("maxIntfDist").doubleValue();
    if(maxIntfDist > 0) {
        return maxIntfDist;
    }

	// the minimum carrier frequency for this cell
	double carrierFrequency = par("carrierFrequency").doubleValue();

	// maximum transmission power possible
	double pMax = par("pMax").doubleValue();
	if (pMax <= 0)
	    throw omnetpp::cRuntimeError("Max transmission power is <= 0!");

	// minimum signal attenuation threshold
	double sat = par("sat").doubleValue();

	// minimum path loss coefficient
	double alpha = par("alpha").doubleValue();

	double waveLength = (BaseWorldUtility::speedOfLight()/carrierFrequency);

	// minimum power level to be able to physically receive a signal
	double minReceivePower = pow(10.0, sat/10.0);

	double interfDistance = pow(waveLength * waveLength * pMax / (16.0*M_PI*M_PI*minReceivePower), 1.0 / alpha);

	return interfDistance;
}

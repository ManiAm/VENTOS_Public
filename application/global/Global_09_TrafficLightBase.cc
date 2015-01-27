
#include <Global_09_TrafficLightBase.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightBase);


TrafficLightBase::~TrafficLightBase()
{

}


void TrafficLightBase::initialize(int stage)
{
    if(stage == 0)
	{
        // get a pointer to the TraCI module
        cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
        TraCI = static_cast<TraCI_Extend *>(module);

    }
}


void TrafficLightBase::finish()
{

}


void TrafficLightBase::handleMessage(cMessage *msg)
{

}


}

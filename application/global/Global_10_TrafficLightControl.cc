
#include <Global_10_TrafficLightControl.h>

namespace VENTOS {

Define_Module(VENTOS::TrafficLightControl);


TrafficLightControl::~TrafficLightControl()
{

}


void TrafficLightControl::initialize(int stage)
{
    TrafficLightBase::initialize(stage);

    if(stage == 0)
	{

    }
}


void TrafficLightControl::finish()
{

}


void TrafficLightControl::handleMessage(cMessage *msg)
{

}


void TrafficLightControl::executeEachTimeStep()
{
    cout << "time: " << simTime().dbl() << ", list of traffic light ids: ";

    list<string> TLlidLst = TraCI->commandGetTLIDList();

    list<string>::iterator it;
    for(it = TLlidLst.begin(); it != TLlidLst.end(); it++)
        cout << *it << " ";

    cout << endl;

    cout << TraCI->commandGetCurrentProgram(string("3")) << endl;

//    uint32_t commandGetCurrentPhase(string TLid);
//    uint32_t commandGetCurrentPhaseDuration(string TLid);
//    uint32_t commandGetNextSwitchTime(string TLid);
//    string commandGetCurrentProgram(string TLid);

}

}

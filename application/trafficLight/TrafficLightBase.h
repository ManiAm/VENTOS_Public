
#ifndef TRAFFICLIGHTBASE_H
#define TRAFFICLIGHTBASE_H

#include <BaseModule.h>
#include <Appl.h>
#include "TraCI_Extend.h"

using namespace std;

namespace VENTOS {

class TrafficLightBase : public BaseModule
{
  public:
      virtual ~TrafficLightBase();
      virtual void finish();
      virtual void initialize(int);
      virtual void handleMessage(cMessage *);
      virtual void receiveSignal(cComponent *, simsignal_t, long);

  protected:
      TraCI_Extend *TraCI;
      simsignal_t Signal_executeFirstTS;
      simsignal_t Signal_executeEachTS;

  protected:
      virtual void executeFirstTimeStep();
      virtual void executeEachTimeStep();
};

}

#endif

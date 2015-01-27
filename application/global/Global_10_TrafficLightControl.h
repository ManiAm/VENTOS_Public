
#ifndef TRAFFICLIGHTCONTROL_H
#define TRAFFICLIGHTCONTROL_H

#include <Global_09_TrafficLightBase.h>
#include <Appl.h>
#include "TraCI_Extend.h"

using namespace std;

namespace VENTOS {

class TrafficLightControl : public TrafficLightBase
{
  public:
      virtual ~TrafficLightControl();
      virtual void finish();
      virtual void initialize(int);
      virtual void handleMessage(cMessage *);

      void executeEachTimeStep();
};

}

#endif

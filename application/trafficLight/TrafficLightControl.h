
#ifndef TRAFFICLIGHTCONTROL_H
#define TRAFFICLIGHTCONTROL_H

#include <TrafficLightBase.h>
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

  private:
      void executeEachTimeStep();

  public:
      int TLControlMode;

  private:
      double nextTime;
      double passTime = 5;
      string prevEdge = "WC";

};

}

#endif

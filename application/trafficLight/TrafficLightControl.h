
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
      void executeFirstTimeStep();
      void executeEachTimeStep();

  public:

  private:
      int TLControlMode;
      double updateInterval;

      list<string> TLlidLst;
      list<string> VehicleLst;
      double nextTime;
      double passTime = 5;
      string prevEdge = "WC";

};

}

#endif
